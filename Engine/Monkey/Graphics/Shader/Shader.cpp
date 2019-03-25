﻿#include "Shader.h"
#include "Engine.h"
#include "spirv_cross.hpp"

#include "Vulkan/VulkanRHI.h"
#include "Vulkan/VulkanMemory.h"
#include "Vulkan/VulkanDevice.h"
#include "File/FileManager.h"

std::unordered_map<std::string, std::shared_ptr<ShaderModule>> Shader::g_ShaderModules;

ShaderModule::~ShaderModule()
{
	if (m_ShaderModule != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule(Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle(), m_ShaderModule, VULKAN_CPU_ALLOCATOR);
		m_ShaderModule = VK_NULL_HANDLE;
	}
    
    if (m_Data)
    {
        delete[] m_Data;
        m_Data = nullptr;
    }
}

std::shared_ptr<ShaderModule> Shader::LoadSPIPVShader(const std::string& filename)
{
	auto it = g_ShaderModules.find(filename);
	if (it != g_ShaderModules.end())
	{
		return it->second;
	}

	uint32 dataSize = 0;
	uint8* dataPtr  = nullptr;
	if (!FileManager::ReadFile(filename, dataPtr, dataSize))
	{
		return nullptr;
	}

	VkShaderModuleCreateInfo moduleCreateInfo;
	ZeroVulkanStruct(moduleCreateInfo, VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO);
	moduleCreateInfo.codeSize = dataSize;
	moduleCreateInfo.pCode    = (uint32_t*)dataPtr;

	VkShaderModule shaderModule;
	VERIFYVULKANRESULT(vkCreateShaderModule(Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle(), &moduleCreateInfo, VULKAN_CPU_ALLOCATOR, &shaderModule));
    
	return std::make_shared<ShaderModule>(shaderModule, (uint32_t*)dataPtr, dataSize);
}

Shader::Shader(std::shared_ptr<ShaderModule> vert, std::shared_ptr<ShaderModule> frag, std::shared_ptr<ShaderModule> geom, std::shared_ptr<ShaderModule> comp, std::shared_ptr<ShaderModule> tesc, std::shared_ptr<ShaderModule> tese)
	: m_InvalidLayout(true)
    , m_InvalidDescSet(true)
	, m_PipelineLayout(VK_NULL_HANDLE)
	, m_DescriptorPool(VK_NULL_HANDLE)
	, m_DescriptorSetLayout(VK_NULL_HANDLE)
	, m_DescriptorSet(VK_NULL_HANDLE)
    , m_Hash(0)
	, m_VertShaderModule(vert)
	, m_FragShaderModule(frag)
	, m_GeomShaderModule(geom)
	, m_CompShaderModule(comp)
	, m_TescShaderModule(tesc)
	, m_TeseShaderModule(tese)
{
    uint32 hash0 = Crc::MakeHashCode(
        vert != nullptr ? vert->GetHash() : 0,
        frag != nullptr ? frag->GetHash() : 0,
        geom != nullptr ? geom->GetHash() : 0
    );
    uint32 hash1 = Crc::MakeHashCode(
        comp != nullptr ? comp->GetHash() : 0,
        tesc != nullptr ? tesc->GetHash() : 0,
        tese != nullptr ? tese->GetHash() : 0
    );
    m_Hash = Crc::MakeHashCode(hash0, hash1);
}

Shader::~Shader()
{
	m_VertShaderModule = nullptr;
	m_FragShaderModule = nullptr;
	m_GeomShaderModule = nullptr;
	m_CompShaderModule = nullptr;
	m_TescShaderModule = nullptr;
	m_TeseShaderModule = nullptr;

    DestroyPipelineLayout();
}

std::shared_ptr<Shader> Shader::Create(const char* vert, const char* frag, const char* geom, const char* compute, const char* tesc, const char* tese)
{
	std::shared_ptr<ShaderModule> vertModule = vert ? LoadSPIPVShader(vert) : nullptr;
	std::shared_ptr<ShaderModule> fragModule = frag ? LoadSPIPVShader(frag) : nullptr;
	std::shared_ptr<ShaderModule> geomModule = geom ? LoadSPIPVShader(geom) : nullptr;
	std::shared_ptr<ShaderModule> tescModule = tesc ? LoadSPIPVShader(tesc) : nullptr;
	std::shared_ptr<ShaderModule> teseModule = tese ? LoadSPIPVShader(tese) : nullptr;
	return std::make_shared<Shader>(vertModule, fragModule, geomModule, tescModule, teseModule);
}

void Shader::DestroyPipelineLayout()
{
	if (m_InvalidLayout)
	{
		return;
	}
	m_InvalidLayout  = true;
    m_InvalidDescSet = true;
    
    VkDevice device = Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle();
    
    vkDestroyDescriptorSetLayout(device, m_DescriptorSetLayout, VULKAN_CPU_ALLOCATOR);
    vkDestroyDescriptorPool(device, m_DescriptorPool, VULKAN_CPU_ALLOCATOR);
    vkDestroyPipelineLayout(device, m_PipelineLayout, VULKAN_CPU_ALLOCATOR);
    
	for (auto it = m_UBInfos.begin(); it != m_UBInfos.end(); ++it)
	{
		const UBInfo& ubInfo = it->second;
		vkFreeMemory(device, ubInfo.memory, VULKAN_CPU_ALLOCATOR);
		vkDestroyBuffer(device, ubInfo.buffer, VULKAN_CPU_ALLOCATOR);
	}
    
	m_PoolSizes.clear();
    m_UBInfos.clear();
	m_ImageInfos.clear();
    m_ShaderStages.clear();
    m_SetLayoutBindings.clear();
	m_VertexInputBindingInfo.Clear();
}

void Shader::CreateUniformBuffer(UBInfo& uniformBuffer, uint32 dataSize, VkBufferUsageFlags usage)
{
    VkDevice device = Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle();
    
	// 创建Buffer
    VkBufferCreateInfo bufferCreateInfo;
    ZeroVulkanStruct(bufferCreateInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
    bufferCreateInfo.size  = dataSize;
    bufferCreateInfo.usage = usage;
    VERIFYVULKANRESULT(vkCreateBuffer(device, &bufferCreateInfo, VULKAN_CPU_ALLOCATOR, &uniformBuffer.buffer));
    
	// 获取内存分配信息
    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(device, uniformBuffer.buffer, &memReqs);
    uint32 memoryTypeIndex = 0;
    Engine::Get()->GetVulkanRHI()->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &memoryTypeIndex);
    
    VkMemoryAllocateInfo allocInfo = {};
    ZeroVulkanStruct(allocInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);
    allocInfo.allocationSize  = memReqs.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;

	// 分配内存并绑定
    VERIFYVULKANRESULT(vkAllocateMemory(device, &allocInfo, VULKAN_CPU_ALLOCATOR, &uniformBuffer.memory));
    VERIFYVULKANRESULT(vkBindBufferMemory(device, uniformBuffer.buffer, uniformBuffer.memory, 0));
    
	// 记录分配信息
	uniformBuffer.size = dataSize;
	uniformBuffer.offset = 0;
    uniformBuffer.allocationSize = uint32(memReqs.size);
}

void Shader::UpdateFragPipelineLayout()
{
	if (m_FragShaderModule == nullptr)
	{
		return;
	}

	VkPipelineShaderStageCreateInfo stageInfo;
	ZeroVulkanStruct(stageInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
	stageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
	stageInfo.module = m_FragShaderModule->GetHandle();
	stageInfo.pName  = "main";
	m_ShaderStages.push_back(stageInfo);

	// 反编译Shader获取相关信息
	spirv_cross::Compiler compiler(m_FragShaderModule->GetData(), m_FragShaderModule->GetDataSize() / sizeof(uint32));
	spirv_cross::ShaderResources resources = compiler.get_shader_resources();

	// 获取Texture
	for (int32 i = 0; i < resources.sampled_images.size(); ++i)
	{
		spirv_cross::Resource& res = resources.sampled_images[i];
		spirv_cross::SPIRType type = compiler.get_type(res.type_id);
		spirv_cross::SPIRType base_type = compiler.get_type(res.base_type_id);
		const std::string &varName = compiler.get_name(res.id);

		VkDescriptorSetLayoutBinding imageBinding = {};
		imageBinding.binding			= compiler.get_decoration(res.id, spv::DecorationBinding);
		imageBinding.descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		imageBinding.descriptorCount	= 1;
		imageBinding.stageFlags			= VK_SHADER_STAGE_FRAGMENT_BIT;
		imageBinding.pImmutableSamplers = nullptr;
		m_SetLayoutBindings.push_back(imageBinding);

		VkDescriptorPoolSize poolSize;
		poolSize.type			 = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSize.descriptorCount = 1;
		m_PoolSizes.push_back(poolSize);

		ImageInfo imageInfo   = {};
		imageInfo.binding	  = m_SetLayoutBindings.size() - 1;
		m_ImageInfos[varName] = imageInfo;
	}
}

void Shader::UpdateCompPipelineLayout()
{
	if (m_CompShaderModule == nullptr)
	{
		return;
	}
	VkPipelineShaderStageCreateInfo stageInfo;
	ZeroVulkanStruct(stageInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
	stageInfo.stage  = VK_SHADER_STAGE_COMPUTE_BIT;
	stageInfo.module = m_CompShaderModule->GetHandle();
	stageInfo.pName  = "main";
	m_ShaderStages.push_back(stageInfo);
}

void Shader::UpdateGeomPipelineLayout()
{
	if (m_GeomShaderModule == nullptr)
	{
		return;
	}
	VkPipelineShaderStageCreateInfo stageInfo;
	ZeroVulkanStruct(stageInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
	stageInfo.stage  = VK_SHADER_STAGE_GEOMETRY_BIT;
	stageInfo.module = m_GeomShaderModule->GetHandle();
	stageInfo.pName  = "main";
	m_ShaderStages.push_back(stageInfo);
}

void Shader::UpdateTescPipelineLayout()
{
	if (m_TescShaderModule == nullptr)
	{
		return;
	}
	VkPipelineShaderStageCreateInfo stageInfo;
	ZeroVulkanStruct(stageInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
	stageInfo.stage  = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	stageInfo.module = m_TescShaderModule->GetHandle();
	stageInfo.pName  = "main";
	m_ShaderStages.push_back(stageInfo);
}

void Shader::UpdateTesePipelineLayout()
{
	if (m_TeseShaderModule == nullptr)
	{
		return;
	}
	VkPipelineShaderStageCreateInfo stageInfo;
	ZeroVulkanStruct(stageInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
	stageInfo.stage  = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	stageInfo.module = m_TeseShaderModule->GetHandle();
	stageInfo.pName  = "main";
	m_ShaderStages.push_back(stageInfo);
}

void Shader::UpdateVertPipelineLayout()
{
    if (m_VertShaderModule == nullptr)
    {
        return;
    }

	// 保存StageInfo
	VkPipelineShaderStageCreateInfo stageInfo;
	ZeroVulkanStruct(stageInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
	stageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
	stageInfo.module = m_VertShaderModule->GetHandle();
	stageInfo.pName  = "main";
	m_ShaderStages.push_back(stageInfo);
    
	// 反编译Shader获取相关信息
    spirv_cross::Compiler compiler(m_VertShaderModule->GetData(), m_VertShaderModule->GetDataSize() / sizeof(uint32));
    spirv_cross::ShaderResources resources = compiler.get_shader_resources();
    
	// 获取Uniform Buffer信息
    for (int32 i = 0; i < resources.uniform_buffers.size(); ++i)
    {
        spirv_cross::Resource& res      = resources.uniform_buffers[i];
        spirv_cross::SPIRType type      = compiler.get_type(res.type_id);
        spirv_cross::SPIRType base_type = compiler.get_type(res.base_type_id);
        const std::string &varName      = compiler.get_name(res.id);
        
        VkDescriptorSetLayoutBinding uboBinding = {};
        uboBinding.binding = compiler.get_decoration(res.id, spv::DecorationBinding);
        uboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboBinding.descriptorCount = 1;
        uboBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboBinding.pImmutableSamplers = nullptr;
        m_SetLayoutBindings.push_back(uboBinding);
        
        VkDescriptorPoolSize poolSize;
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = 1;
        m_PoolSizes.push_back(poolSize);
        
        UBInfo ubInfo;
        CreateUniformBuffer(ubInfo, uint32(compiler.get_declared_struct_size(base_type)), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		ubInfo.binding = m_SetLayoutBindings.size() - 1;
		ubInfo.bufferInfo.buffer = ubInfo.buffer;
		ubInfo.bufferInfo.offset = 0;
		ubInfo.bufferInfo.range  = ubInfo.size;
		m_UBInfos[varName] = ubInfo;
    }
    
	// 获取Input Location信息
    for (int32 i = 0; i < resources.stage_inputs.size(); ++i)
    {
        spirv_cross::Resource& res = resources.stage_inputs[i];
        const std::string &varName = compiler.get_name(res.id);
        VertexAttribute attribute  = StringToVertexAttribute(varName.c_str());
		m_VertexInputBindingInfo.AddBinding(attribute, compiler.get_decoration(res.id, spv::DecorationLocation));
    }

	m_VertexInputBindingInfo.Update();
}

void Shader::UpdatePipelineLayout()
{
	if (!m_InvalidLayout) 
	{
		return;
	}

    DestroyPipelineLayout();

    UpdateVertPipelineLayout();
	UpdateGeomPipelineLayout();
	UpdateTescPipelineLayout();
	UpdateTesePipelineLayout();
	UpdateCompPipelineLayout();
    UpdateFragPipelineLayout();
    
    VkDevice device = Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle();
    
    // 创建SetLayout
    VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo;
    ZeroVulkanStruct(setLayoutCreateInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
    setLayoutCreateInfo.bindingCount = uint32_t(m_SetLayoutBindings.size());
    setLayoutCreateInfo.pBindings    = m_SetLayoutBindings.data();
    VERIFYVULKANRESULT(vkCreateDescriptorSetLayout(device, &setLayoutCreateInfo, VULKAN_CPU_ALLOCATOR, &m_DescriptorSetLayout));
    
    // 创建Pool
    VkDescriptorPoolCreateInfo poolCreateInfo;
    ZeroVulkanStruct(poolCreateInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);
	poolCreateInfo.maxSets       = 1;
    poolCreateInfo.poolSizeCount = uint32_t(m_PoolSizes.size());
    poolCreateInfo.pPoolSizes    = m_PoolSizes.data();
    VERIFYVULKANRESULT(vkCreateDescriptorPool(device, &poolCreateInfo, VULKAN_CPU_ALLOCATOR,  &m_DescriptorPool));
    
    // 分配Set
    VkDescriptorSetAllocateInfo setAllococateInfo;
    ZeroVulkanStruct(setAllococateInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO);
    setAllococateInfo.descriptorPool     = m_DescriptorPool;
    setAllococateInfo.descriptorSetCount = 1;
    setAllococateInfo.pSetLayouts        = &m_DescriptorSetLayout;
    VERIFYVULKANRESULT(vkAllocateDescriptorSets(device, &setAllococateInfo, &m_DescriptorSet));
    
	// 创建PipelineLayout
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
    ZeroVulkanStruct(pipelineLayoutCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO);
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts    = &m_DescriptorSetLayout;
    VERIFYVULKANRESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, VULKAN_CPU_ALLOCATOR, &m_PipelineLayout));

	m_InvalidLayout  = false;
    m_InvalidDescSet = true;
}

void Shader::UpdateDescriptorSet()
{
	if (!m_InvalidDescSet) {
		return;
	}
	m_InvalidDescSet = false;

	// 更新Descriptor Set
	std::vector<VkWriteDescriptorSet> descriptorWrites(m_SetLayoutBindings.size());
	int32 index = 0;

	for (auto it = m_UBInfos.begin(); it != m_UBInfos.end(); ++it)
	{
		const UBInfo& ubInfo = it->second;
		ZeroVulkanStruct(descriptorWrites[index], VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
		descriptorWrites[index].dstSet			= m_DescriptorSet;
		descriptorWrites[index].dstBinding		= m_SetLayoutBindings[ubInfo.binding].binding;
		descriptorWrites[index].dstArrayElement = 0;
		descriptorWrites[index].descriptorType	= m_SetLayoutBindings[ubInfo.binding].descriptorType;
		descriptorWrites[index].descriptorCount = 1;
		descriptorWrites[index].pBufferInfo		= &(ubInfo.bufferInfo);
		index += 1;
	}
    
    // ImageInfo
	for (auto it = m_ImageInfos.begin(); it != m_ImageInfos.end(); ++it)
	{
		std::shared_ptr<TextureBase> texture    = m_Texturess[it->first];
		const ImageInfo& imageInfo = it->second;
		ZeroVulkanStruct(descriptorWrites[index], VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
		descriptorWrites[index].dstSet			= m_DescriptorSet;
		descriptorWrites[index].dstBinding		= m_SetLayoutBindings[imageInfo.binding].binding;
		descriptorWrites[index].dstArrayElement = 0;
		descriptorWrites[index].descriptorType	= m_SetLayoutBindings[imageInfo.binding].descriptorType;
		descriptorWrites[index].descriptorCount = 1;
		descriptorWrites[index].pImageInfo		= &(texture->GetImageInfo());
		index += 1;
	}
	
    VkDevice device = Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle();
    vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, VULKAN_CPU_ALLOCATOR);
}

void Shader::SetTextureData(const std::string& name, std::shared_ptr<TextureBase> texture)
{
    m_Texturess[name] = texture;
    m_InvalidDescSet  = true;
}

void Shader::SetUniformData(const std::string& name, uint8* dataPtr, uint32 dataSize)
{
	UBDataInfo ubDataInfo = {};
	ubDataInfo.dataPtr    = dataPtr;
	ubDataInfo.dataSize   = dataSize;

	m_UBDatas[name] = ubDataInfo;
	
	auto it = m_UBInfos.find(name);
	if (it == m_UBInfos.end()) 
	{
		return;	
	}
	
    VkDevice device = Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle();
    UBInfo& ubInfo  = it->second;
    uint8_t *pData  = nullptr;
    VERIFYVULKANRESULT(vkMapMemory(device, ubInfo.memory, 0, dataSize, 0, (void**)&pData));
    std::memcpy(pData, dataPtr, dataSize);
    vkUnmapMemory(device, ubInfo.memory);
}