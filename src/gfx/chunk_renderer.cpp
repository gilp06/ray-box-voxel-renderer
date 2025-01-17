#include <gfx/chunk_renderer.hpp>
#include <iostream>
#include <tracy/Tracy.hpp>

ChunkRenderer::ChunkRenderer(World &world) : world(world)
{
    shader = new ShaderProgram("resources/shaders/chunk.vert.glsl", "resources/shaders/chunk.frag.glsl");
    shader->Use();
    shader->SetUniformBlock("ubo", 0);
    shader->SetUniformUInt("CHUNK_SIZE", (uint32_t)CHUNK_SIZE);

    world.SubscribeToChunkLoad([this](const glm::ivec3 &position)
                               { this->OnChunkLoad(position); });
    world.SubscribeToChunkUnload([this](const glm::ivec3 &position)
                                 { this->OnChunkUnload(position); });
    world.SubscribeToChunkUpdate([this](const glm::ivec3 &position)
                                 { this->OnChunkUpdate(position); });
}

ChunkRenderer::~ChunkRenderer()
{
    delete shader;
}

void ChunkRenderer::AddChunk(glm::ivec3 position)
{
    if (chunks.find(position) != chunks.end())
    {
        return;
    }

    chunks.try_emplace(position, std::make_shared<GPUChunk>(position, world));
}

void ChunkRenderer::RemoveChunk(glm::ivec3 position)
{
    if (chunks.find(position) == chunks.end())
    {
        return;
    }

    chunks.extract(position);
}

void ChunkRenderer::Render(Camera &camera)
{
    glm::mat4 frustum_space = camera.GetProjectionMatrix() * camera.GetViewMatrix();
    for (auto& data : chunks)
    {

        shader->SetUniformIVec3("chunk_pos", data.first);

        

        if (!within_frustum(glm::vec3(data.first) * (float)CHUNK_SIZE, frustum_space))
        {
            continue;
        }
        // std::cout << "Rendering chunk at " << position.x << " " << position.y << " " << position.z << std::endl;
        // std::cout << "Chunk size: " << chunk.size << std::endl;
        glBindVertexArray(data.second.get()->vao);
        glBindBuffer(GL_ARRAY_BUFFER, data.second.get()->vbo);
        glDrawArrays(GL_POINTS, 0, data.second.get()->size);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
}

void ChunkRenderer::Update()
{
}

void ChunkRenderer::OnChunkLoad(const glm::ivec3 &position)
{
    AddChunk(position);
}

void ChunkRenderer::OnChunkUnload(const glm::ivec3 &position)
{
    RemoveChunk(position);
}

void ChunkRenderer::OnChunkUpdate(const glm::ivec3 &position)
{
    RemoveChunk(position);
    AddChunk(position);
}

GPUChunk::GPUChunk(glm::ivec3 position, World &w)
{
    ZoneScoped;
    std::vector<ChunkBufferItem> buffer;
    Chunk &chunk = w.GetChunk(position);

    for (int i = 0; i < CHUNK_VOLUME; i++)
    {
        uint8_t block = chunk.GetBlocks()[i];
        BlockData &block_data = BlockManager::GetBlockData(block);
        if (block_data.type == BlockType::Empty)
        {
            continue;
        }

        glm::uvec3 pos;
        pos.x = deinterleaveBits(i, 2);
        pos.y = deinterleaveBits(i, 1);
        pos.z = deinterleaveBits(i, 0);

        constexpr glm::ivec3 directions[] = {
            glm::ivec3(1, 0, 0),
            glm::ivec3(-1, 0, 0),
            glm::ivec3(0, 1, 0),
            glm::ivec3(0, -1, 0),
            glm::ivec3(0, 0, 1),
            glm::ivec3(0, 0, -1),
        };
        bool visible = false;

        for (auto &dir : directions)
        {
            glm::ivec3 new_pos = glm::ivec3(pos) + dir;
            if (new_pos.x < 0 || new_pos.x >= CHUNK_SIZE ||
                new_pos.y < 0 || new_pos.y >= CHUNK_SIZE ||
                new_pos.z < 0 || new_pos.z >= CHUNK_SIZE)
            {
                // special case for blocks on the edge of the chunk
                // get chunk in direction of dir
                glm::ivec3 chunk_pos = position + dir;
                if (w.ChunkLoaded(chunk_pos))
                {
                    // std::cout << "Attempted accessing neighbor chunk" << std::endl;
                    Chunk &neighbor_chunk = w.GetChunk(chunk_pos);
                    // std::cout << "Got neighbor chunk" << std::endl;
                    glm::ivec3 local_pos = glm::ivec3(new_pos);
                    if (new_pos.x < 0)
                        local_pos.x += CHUNK_SIZE;
                    if (new_pos.x >= CHUNK_SIZE)
                        local_pos.x -= CHUNK_SIZE;
                    if (new_pos.y < 0)
                        local_pos.y += CHUNK_SIZE;
                    if (new_pos.y >= CHUNK_SIZE)
                        local_pos.y -= CHUNK_SIZE;
                    if (new_pos.z < 0)
                        local_pos.z += CHUNK_SIZE;
                    if (new_pos.z >= CHUNK_SIZE)
                        local_pos.z -= CHUNK_SIZE;

                    // std::cout << "Local pos: " << local_pos.x << " " << local_pos.y << " " << local_pos.z << std::endl;

                    uint8_t neighbor_block = neighbor_chunk.GetBlock(local_pos.x, local_pos.y, local_pos.z);
                    // std::cout << "Got neighbor block" << std::endl;
                    if (BlockManager::GetBlockData(neighbor_block).type == BlockType::Empty)
                    {
                        // std::cout << "Neighbor block is empty" << std::endl;
                        // std::cout << "visible due to neighbor block" << std::endl;
                        visible = true;
                        break;
                    }
                    // std::cout << "Neighbor block is not empty" << std::endl;
                }
            }
            else
            {
                // std::cout << "Checked neighboring chunk" << std::endl;
                uint8_t neighbor_block = chunk.GetBlock(new_pos.x, new_pos.y, new_pos.z);

                if (BlockManager::GetBlockData(neighbor_block).type == BlockType::Empty)
                {
                    // std::cout << new_pos.x << " " << new_pos.y << " " << new_pos.z << std::endl;
                    // std::cout << "visible due to neighbor block" << std::endl;
                    visible = true;
                    break;
                }
            }
        }

        if (!visible)
        {
            continue;
        }

        // std::cout << "Visible block at " << pos.x << " " << pos.y << " " << pos.z << std::endl;

        ChunkBufferItem item;
        item.position = pos;
        item.color = block_data.color;

        buffer.push_back(item);
    }

    size = buffer.size();

    // glGenBuffers(1, &vbo);
    // glGenVertexArrays(1, &vao);
    // glBindVertexArray(vao);
    // glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(ChunkBufferItem), buffer.data(), GL_STATIC_DRAW);
    // glVertexAttribIPointer(0, 3, GL_UNSIGNED_INT, sizeof(ChunkBufferItem), (void *)offsetof(ChunkBufferItem, position));
    // glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(ChunkBufferItem), (void *)offsetof(ChunkBufferItem, color));
    // glEnableVertexAttribArray(0);
    // glEnableVertexAttribArray(1);
    // glBindBuffer(GL_ARRAY_BUFFER, 0);
    // glBindVertexArray(0);

    glCreateBuffers(1, &vbo);
    glCreateVertexArrays(1, &vao);
    glNamedBufferStorage(vbo, buffer.size() * sizeof(ChunkBufferItem), buffer.data(), GL_DYNAMIC_STORAGE_BIT);
    glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(ChunkBufferItem));
    glVertexArrayAttribIFormat(vao, 0, 3, GL_UNSIGNED_INT, offsetof(ChunkBufferItem, position));
    glVertexArrayAttribIFormat(vao, 1, 1, GL_UNSIGNED_INT, offsetof(ChunkBufferItem, color));
    glEnableVertexArrayAttrib(vao, 0);
    glEnableVertexArrayAttrib(vao, 1);
    glVertexArrayAttribBinding(vao, 0, 0);
    glVertexArrayAttribBinding(vao, 1, 0);
    glBindVertexArray(0);
}

GPUChunk::~GPUChunk()
{
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}
