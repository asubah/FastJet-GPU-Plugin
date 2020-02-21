CUDA_BASE:=/usr/local/cuda
CUDA_INCLUDE_PATH=$(CUDA_BASE)/include
CUDA_LIBRARY_PATH=$(CUDA_BASE)/lib64

CXX=g++
CXX_FLAGS=-std=c++17 -O2 -g -I$(CUDA_INCLUDE_PATH)
LD_FLAGS=-L$(CUDA_LIBRARY_PATH) -lcudart -lcuda

NVCC=$(CUDA_BASE)/bin/nvcc -ccbin $(CXX)
NVCC_FLAGS=-std=c++14 -O2 --expt-relaxed-constexpr --expt-extended-lambda -g --generate-line-info --source-in-ptx --generate-code arch=compute_50,code=sm_50

FASTJET_CONF:= `fastjet-install/bin/fastjet-config --cxxflags --libs --plugins`
FASTJET_INCLUDE:= -Ifastjet-install/include
FASTJET_GPU_PLUGIN:= -Ifastjet-3.3.2/plugins/GPUPlugin

.PHONY: all clean

all: test

clean:
	rm -f *.o test

grid.o: fastjet-3.3.2/plugins/GPUPlugin/cuda/grid.cu
	$(NVCC) $(NVCC_FLAGS) $(FASTJET_INCLUDE) -c $< -o $@

test.o: test.cc
	$(CXX) $(CXX_FLAGS) $(FASTJET_CONF) $(FASTJET_GPU_PLUGIN) $(LD_FLAGS) -c $< -o $@

GPUPlugin.o: fastjet-3.3.2/plugins/GPUPlugin/GPUPlugin.cc
	$(CXX) $(CXX_FLAGS) $(FASTJET_CONF) $(FASTJET_GPU_PLUGIN) $(LD_FLAGS) -c $< -o $@

test: test.o GPUPlugin.o grid.o
	$(CXX) $(CXX_FLAGS) $^ $(FASTJET_CONF) $(FASTJET_GPU_PLUGIN) $(LD_FLAGS) -o $@
