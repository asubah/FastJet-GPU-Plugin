#ifndef GPUPlugin_hh
#define GPUPlugin_hh
#include "fastjet/ClusterSequence.hh"
#include "fastjet/JetDefinition.hh"
#include "../cuda/GPUPseudoJet.h"

#include <vector>

FASTJET_BEGIN_NAMESPACE

class GPUPlugin : public JetDefinition::Plugin{
public:
  GPUPlugin(double r, fastjet::JetAlgorithm a) : radius(r), algo(a) {}

  virtual std::string description () const {return "GPU Clustring Algorithm."; }
  virtual void run_clustering(ClusterSequence &) const;

  virtual double R() const {return radius;}

private:

  double radius;
  JetAlgorithm algo;
};

FASTJET_END_NAMESPACE
#endif  // GPUPlugin_hh
