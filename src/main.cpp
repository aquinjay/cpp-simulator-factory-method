#include <iostream>
#include <random>
#include <cmath>
#include <fstream>
#include <functional>
#include <memory>
#include <vector>

class MHSimulator {
 public:
  virtual ~MHSimulator() {}
  virtual double proposer() = 0;
  virtual void runner() = 0;
  virtual void viewer() = 0;

  const std::vector<double>& getSimOutput() const {
    return sim_output;
  }

 protected:
  std::vector<double> sim_output;
};

class GenericMHSimulator : public MHSimulator {
 public:
  GenericMHSimulator(int n, std::function<double(double)> target_density_func)
    : N(n), target_density(target_density_func), gen(std::random_device {}()), current_x(uniform(gen)) {
    sim_output.resize(N); // Resize the vector to N elements
  }

  double proposer() override {
    return current_x + proposal_width * (uniform(gen) - 0.5);
  }

  // Sampling loop
  void runner() override {
    for (int i = 0; i < N; ++i) {
      // Propose a new state
      double proposal_x = proposer();
      double acceptance_ratio = target_density(proposal_x) / target_density(current_x);

      // Accept or reject the proposal
      if (uniform(gen) < acceptance_ratio) {
         current_x = proposal_x;
       }

      sim_output.push_back(current_x);
    }
  }

  void viewer() override {
    for (double value : getSimOutput()) {
      std::cout << value << std::endl;
    }
  }

 private:
  std::function<double(double)> target_density;
  // MH parameters
  int N; // Number of samples
  double proposal_width = 0.5; // Width of the uniform proposal distribution

  // Initial state
  double current_x;
  std::mt19937 gen;
  std::uniform_real_distribution<> uniform{0, 1};
};

class MHCreator {
 public:
  MHCreator(std::function<double(double)> target_density_func) 
    : target_density(target_density_func) {}
  virtual ~MHCreator() {}
  virtual std::unique_ptr<MHSimulator> createSimulator() const = 0;

  void OperatorRunner() const {
    std::unique_ptr<MHSimulator> FactoryMethods = this->createSimulator(); // overwritten by concreteCreator's simulator initiator
    FactoryMethods->runner(); // runs the concrete product's runner
    FactoryMethods->viewer();
  }

protected:
  std::function<double(double)> target_density;
};

class GenericMHSimulatorCreator : public MHCreator {
public:
  GenericMHSimulatorCreator(std::function<double(double)> target_density_func)
    : MHCreator(target_density_func) {}

  std::unique_ptr<MHSimulator> createSimulator() const override {
    return std::make_unique<GenericMHSimulator>(10000, target_density); // returning simulator
  }
};

void ClientCode(const MHCreator& creator) {
  creator.OperatorRunner(); // runs creator operatorRunner which runs the concreate creator's simulator.
}

int main() {
  auto target_density =[](double x) {
    return std::pow(x, 2.6 - 1) * std::pow(1 - x, 6.3 - 1);
  };
  GenericMHSimulatorCreator creator(target_density);
  ClientCode(creator);
}
