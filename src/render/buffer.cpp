#include "buffer.hpp"
#include "command.hpp"
#include "common.hpp"
#include "utils/assert.hpp"

#include <fmt/format.h>

namespace shady::render {

void
Buffer::Map(VkDeviceSize size)
{
   vkMapMemory(Data::vk_device, m_bufferMemory, 0, size, 0, &m_mappedMemory);
   m_mapped = true;
}

void
Buffer::Unmap()
{
   if (m_mapped)
   {
      vkUnmapMemory(Data::vk_device, m_bufferMemory);
      m_mapped = false;
      m_mappedMemory = nullptr;
   }
}

void
Buffer::CopyData(const void* data)
{
   utils::Assert(m_mapped, "Buffer is not mapped!");
   memcpy(m_mappedMemory, data, m_bufferSize);
}

void
Buffer::CopyDataWithStaging(void* data, size_t dataSize)
{
   VkBuffer stagingBuffer;
   VkDeviceMemory stagingBufferMemory;
   Buffer::CreateBuffer(dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        stagingBuffer, stagingBufferMemory);

   void* mapped_data;
   vkMapMemory(Data::vk_device, stagingBufferMemory, 0, dataSize, 0, &mapped_data);
   memcpy(mapped_data, data, dataSize);
   vkUnmapMemory(Data::vk_device, stagingBufferMemory);

   Buffer::CopyBuffer(stagingBuffer, m_buffer, dataSize);
}

void
Buffer::CopyDataToImageWithStaging(VkImage image, void* data, size_t dataSize,
                                   const std::vector< VkBufferImageCopy >& copyRegions)
{
   VkBuffer stagingBuffer;
   VkDeviceMemory stagingBufferMemory;
   Buffer::CreateBuffer(dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        stagingBuffer, stagingBufferMemory);

   void* mapped_data;
   vkMapMemory(Data::vk_device, stagingBufferMemory, 0, dataSize, 0, &mapped_data);
   memcpy(mapped_data, data, dataSize);
   vkUnmapMemory(Data::vk_device, stagingBufferMemory);

   VkCommandBuffer commandBuffer = Command::BeginSingleTimeCommands();

   vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          static_cast<uint32_t>(copyRegions.size()), copyRegions.data());

   Command::EndSingleTimeCommands(commandBuffer);

   vkDestroyBuffer(Data::vk_device, stagingBuffer, nullptr);
   vkFreeMemory(Data::vk_device, stagingBufferMemory, nullptr);
}

void
Buffer::SetupDescriptor(VkDeviceSize /*size*/, VkDeviceSize offset)
{
   m_descriptor.offset = offset;
   m_descriptor.buffer = m_buffer;
   m_descriptor.range = m_bufferSize;
}

void
AllocateMemory(VkMemoryRequirements memReq, VkDeviceMemory& bufferMemory,
               VkMemoryPropertyFlags properties)
{
   VkMemoryAllocateInfo allocInfo{};
   allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
   allocInfo.allocationSize = memReq.size;
   allocInfo.memoryTypeIndex = FindMemoryType(memReq.memoryTypeBits, properties);

   VK_CHECK(vkAllocateMemory(Data::vk_device, &allocInfo, nullptr, &bufferMemory),
            "failed to allocate buffer memory!");
}

void
Buffer::AllocateImageMemory(VkImage image, VkDeviceMemory& bufferMemory,
                            VkMemoryPropertyFlags properties)
{
   VkMemoryRequirements memRequirements;
   vkGetImageMemoryRequirements(Data::vk_device, image, &memRequirements);

   AllocateMemory(memRequirements, bufferMemory, properties);
}

void
Buffer::AllocateBufferMemory(VkBuffer buffer, VkDeviceMemory& bufferMemory,
                             VkMemoryPropertyFlags properties)
{
   VkMemoryRequirements memRequirements;
   vkGetBufferMemoryRequirements(Data::vk_device, buffer, &memRequirements);

   AllocateMemory(memRequirements, bufferMemory, properties);
}

Buffer
Buffer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
   Buffer newBuffer;
   newBuffer.m_bufferSize = size;
   CreateBuffer(size, usage, properties, newBuffer.m_buffer, newBuffer.m_bufferMemory);
   newBuffer.SetupDescriptor();

   return newBuffer;
}

void
Buffer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                     VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
   VkBufferCreateInfo bufferInfo{};
   bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
   bufferInfo.size = size;
   bufferInfo.usage = usage;
   bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

   VK_CHECK(vkCreateBuffer(Data::vk_device, &bufferInfo, nullptr, &buffer),
            "failed to create buffer!");

   AllocateBufferMemory(buffer, bufferMemory, properties);

   vkBindBufferMemory(Data::vk_device, buffer, bufferMemory, 0);
}

void
Buffer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
   VkCommandBuffer commandBuffer = Command::BeginSingleTimeCommands();

   VkBufferCopy copyRegion{};
   copyRegion.size = size;
   vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

   Command::EndSingleTimeCommands(commandBuffer);
}

void
Buffer::Flush(VkDeviceSize size, VkDeviceSize offset) const
{
   VkMappedMemoryRange mappedRange = {};
   mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
   mappedRange.memory = m_bufferMemory;
   mappedRange.offset = offset;
   mappedRange.size = size;

   VK_CHECK(vkFlushMappedMemoryRanges(Data::vk_device, 1, &mappedRange), "Buffer::Flush error!");
}

void
Buffer::Destroy()
{
   if (m_buffer)
   {
      vkDestroyBuffer(Data::vk_device, m_buffer, nullptr);
   }
   if (m_bufferMemory)
   {
      vkFreeMemory(Data::vk_device, m_bufferMemory, nullptr);
   }
}

} // namespace shady::render
