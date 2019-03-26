#pragma once

#define __CL_ENABLE_EXCEPTIONS 
#define CL_USE_DEPRECATED_OPENCL_2_0_APIS

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#include "cl.hpp"
#pragma clang diagnostic pop
#else
#include "cl.hpp"
#endif

#include <boost/optional.hpp>
#include <time.h>
#include <functional>
#include <libethash/ethash.h>

class ethash_cl_miner
{
public:
	struct search_hook
	{
		virtual ~search_hook(); // always a virtual destructor for a class with virtuals.

		// reports progress, return true to abort
		virtual bool found(uint64_t const* nonces, uint32_t count) = 0;
		virtual bool searched(uint64_t start_nonce, uint32_t count) = 0;
	};

public:
	ethash_cl_miner();
	~ethash_cl_miner();

	static bool searchForAllDevices(unsigned _platformId, std::function<bool(cl::Device const&)> _callback);
	static bool searchForAllDevices(std::function<bool(cl::Device const&)> _callback);
	static void doForAllDevices(unsigned _platformId, std::function<void(cl::Device const&)> _callback);
	static void doForAllDevices(std::function<void(cl::Device const&)> _callback);
	static unsigned getNumPlatforms();
	static unsigned getNumDevices(unsigned _platformId = 0);
	static std::string platform_info(unsigned _platformId = 0, unsigned _deviceId = 0);
	static void listDevices();
	static bool configureGPU(
		bool _allowCPU,
		unsigned _extraGPUMemory,
		bool _forceSingleChunk,
		boost::optional<uint64_t> _currentBlock
	);

	bool init(
		uint8_t const* _dag,
		uint64_t _dagSize,
		unsigned workgroup_size = 64,
		unsigned _platformId = 0,
		unsigned _deviceId = 0
	);
	void finish();
	void search(uint8_t const* header, uint64_t target, search_hook& hook);

	void hash_chunk(uint8_t* ret, uint8_t const* header, uint64_t nonce, unsigned count);
	void search_chunk(uint8_t const* header, uint64_t target, search_hook& hook);

private:

	static std::vector<cl::Device> getDevices(std::vector<cl::Platform> const& _platforms, unsigned _platformId);
	
	enum { c_max_search_results = 63, c_num_buffers = 2, c_hash_batch_size = 1024, c_search_batch_size = 1024*256 };

	cl::Context m_context;
	cl::CommandQueue m_queue;
	cl::Kernel m_hash_kernel;
	cl::Kernel m_search_kernel;
	unsigned int m_dagChunksNum;
	std::vector<cl::Buffer> m_dagChunks;
	cl::Buffer m_header;
	cl::Buffer m_hash_buf[c_num_buffers];
	cl::Buffer m_search_buf[c_num_buffers];
	unsigned m_workgroup_size;
	bool m_opencl_1_1;

	/// Force dag upload to GPU in a single chunk even if OpenCL thinks you can't do it. Use at your own risk.
	static bool s_forceSingleChunk;
	/// Allow CPU to appear as an OpenCL device or not. Default is false
	static bool s_allowCPU;
	/// GPU memory required for other things, like window rendering e.t.c.
	/// User can set it via the --cl-extragpu-mem argument.
	static unsigned s_extraRequiredGPUMem;
};
