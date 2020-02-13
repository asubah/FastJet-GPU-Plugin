// fastjet stuff
#include "fastjet/ClusterSequence.hh"

// Plugin headers ./plugins/GPUPlugin/
#include "fastjet/GPUPlugin.hh"
// CUDA
#include "cuda/GPUPseudoJet.h"
#include "cuda/cudaCheck.h"
#include "cuda/cluster.h"

// other stuff
#include <vector>
#include <cuda_runtime.h>

FASTJET_BEGIN_NAMESPACE

using namespace std;

void initialise() {
  cudaSetDevice(0);
  cudaDeviceProp prop;
  cudaGetDeviceProperties(&prop, 0);
  std::cout << "Running on CUDA device " << prop.name << std::endl;
  int value;
  cudaDeviceGetAttribute(&value, cudaDevAttrMaxSharedMemoryPerBlock, 0);
  std::cout << "  - maximum shared memory per block: " << value / 1024 << " kB" << std::endl;

  cudaCheck(cudaDeviceSetLimit(cudaLimitPrintfFifoSize, 10 * 1024 * 1024));
  size_t size;
  cudaCheck(cudaDeviceGetLimit(&size, cudaLimitPrintfFifoSize));
  std::cout << "  - kernel printf buffer size:       " << size / 1024 << " kB" << std::endl;
}

void GPUPlugin::run_clustering(ClusterSequence & cs) const {
  // initialise the GPU
  initialise();
  
  std::vector<GPUPseudoJet> gpu_particles;
  for (int i = 0; i < cs.jets().size(); ++i) {
    gpu_particles.push_back({
          i, false,
          cs.jets()[i].px(),
          cs.jets()[i].py(),
          cs.jets()[i].pz(),
          cs.jets()[i].E()
        });
  }

  // allocate GPU memory for the input particles
  GPUPseudoJet* particles_d;
  cudaCheck(cudaMalloc(&particles_d, sizeof(GPUPseudoJet) * gpu_particles.size()));

  // copy the input to the GPU
  cudaCheck(cudaMemcpy(particles_d, gpu_particles.data(), sizeof(GPUPseudoJet) *
        gpu_particles.size(), cudaMemcpyDefault));
      
  cudaEvent_t start, stop;
  cudaCheck(cudaEventCreate(&start));
  cudaCheck(cudaEventCreate(&stop));

  // run the clustering algorithm and measure its running time
  cudaCheck(cudaEventRecord(start));
  GPUCluster::cluster(particles_d, gpu_particles.size(), (Algorithm) algo, radius);
  
  cudaCheck(cudaEventRecord(stop));
  cudaCheck(cudaEventSynchronize(stop));

  float milliseconds;
  cudaCheck(cudaEventElapsedTime(&milliseconds, start, stop));
  cout << "time: " << milliseconds << "ms" << endl;
  // copy the clustered jets back to the CPU
  cudaCheck(cudaMemcpy(gpu_particles.data(), particles_d, sizeof(GPUPseudoJet) * gpu_particles.size(), cudaMemcpyDefault));

  // free GPU memory
  cudaCheck(cudaFree(particles_d));
 
  // Reports the jets back to the ClusterSequence object
  for (int i = 0; i < gpu_particles.size(); ++i) {
    if (gpu_particles[i].isJet) {
      fastjet::PseudoJet jet(
              gpu_particles[i].px,
              gpu_particles[i].py,
              gpu_particles[i].pz,
              gpu_particles[i].E
          );

      // Create a pointer to overcome the const function
      PseudoJet * ptrJet = const_cast<PseudoJet*>(&cs.jets()[i]);

      // Use the pointer to modify the protected jet
      (*ptrJet).reset_momentum(jet);

      // Record the jet in the ClusterSequence
      cs.plugin_record_iB_recombination(i, jet.beam_distance());
    }
  }
}

FASTJET_END_NAMESPACE
