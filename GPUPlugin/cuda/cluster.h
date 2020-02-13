#ifndef cluster_h
#define cluster_h

enum class Algorithm { Kt = 1, CambridgeAachen = 0, AntiKt = 2};

class GPUCluster {
public:
  static void cluster(GPUPseudoJet *particles, int size, Algorithm algo, double r);
};
#endif  // cluster_h
