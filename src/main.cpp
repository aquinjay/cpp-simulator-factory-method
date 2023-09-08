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
  virtual double proposal_func() = 0;

  MHSimulator(int n, std::function<double(double)> target_density_func)
    : N(n), target_density(target_density_func),
    gen(std::random_device{}()), current_x(uniform(gen)) {}

  // Sampling loop
  std::vector<double> runner() {
    std::vector<double> results;
    results.reserve(N);
    for (int i = 0; i < N; ++i) {
      // Propose a new state
      double proposal_x = proposal_func();
      double acceptance_ratio = target_density(proposal_x) / target_density(current_x);

      // Accept or reject the proposal
      if (uniform(gen) < acceptance_ratio) {
         current_x = proposal_x;
       }

      results.push_back(current_x);
    }
    return results;
  }

  protected:
  std::function<double(double)> target_density;
  std::mt19937 gen;
  std::uniform_real_distribution<> uniform{0, 1};
  int N;  // Number of samples
  double current_x;
};

class GenericMHSimulator : public MHSimulator {
 public:
  GenericMHSimulator(int n, std::function<double(double)> target_density_func)  // Pass chain size and target density function
    : MHSimulator(n, target_density_func) {}

  double proposal_func() override {
    return current_x + proposal_width * (uniform(gen) - 0.5);
  }
 private:
  double proposal_width = 0.5;
};

// Independent class

class IndependentMHSimulator : public MHSimulator {
 public:
  IndependentMHSimulator(int n, std::function<double(double)> target_density_func)  // Pass chain size and target density function
    : MHSimulator(n, target_density_func) {}

  double proposal_func() override {
    return proposal_width * (uniform(gen) - 0.5);
  }

 private:
  double proposal_width = 0.5; // Width of the uniform proposal distribution
};


class RandomWalkMHSimulator : public MHSimulator {
 public:
  RandomWalkMHSimulator(int n, std::function<double(double)> target_density_func)  // Pass chain size and target density function
    : MHSimulator(n, target_density_func) {}

  double proposal_func() override {
    return current_x +  (uniform(gen) - 0.5);
  }

 private:
  double proposal_width = 0.5; // Width of the uniform proposal distribution
};

// Creator class logic

class MHCreator {
 public:
  MHCreator(int n, std::function<double(double)> target_density_func)
    : N(n), target_density(target_density_func) {}
  virtual ~MHCreator() {}
  virtual std::unique_ptr<MHSimulator> createSimulator() const = 0;

  std::vector<double> OperatorRunner() const {
    std::unique_ptr<MHSimulator> simulator = this->createSimulator(); // overwritten by concreteCreator's simulator initiator
    return simulator->runner(); // runs the concrete product's runner
  }

protected:
  std::function<double(double)> target_density;
  int N;
};

class GenericMHSimulatorCreator : public MHCreator {
public:
  GenericMHSimulatorCreator(int n, std::function<double(double)> target_density_func)
    : MHCreator(n, target_density_func) {}

  std::unique_ptr<MHSimulator> createSimulator() const override {
    return std::make_unique<GenericMHSimulator>(N, target_density); // returning simulator
  }
};

class IndependentMHSimulatorCreator : public MHCreator {
public:
  IndependentMHSimulatorCreator(int n, std::function<double(double)> target_density_func)
    : MHCreator(n, target_density_func) {}

  std::unique_ptr<MHSimulator> createSimulator() const override {
    return std::make_unique<IndependentMHSimulator>(N, target_density); // returning simulator
  }
};

class RandomWalkMHSimulatorCreator : public MHCreator {
public:
  RandomWalkMHSimulatorCreator(int n, std::function<double(double)> target_density_func)
    : MHCreator(n, target_density_func) {}

  std::unique_ptr<MHSimulator> createSimulator() const override {
    return std::make_unique<RandomWalkMHSimulator>(N, target_density); // returning simulator
  }
};


void ClientCode(const MHCreator& creator) {
  std::vector<double> results = creator.OperatorRunner(); // runs creator operatorRunner which runs the concreate creator's simulator.
  for (double value : results) {
    std::cout << value << std::endl;
  }
}

int main() {
  auto target_density =[](double x) {
    return std::pow(x, 2.6 - 1) * std::pow(1 - x, 6.3 - 1);
  };
  GenericMHSimulatorCreator creator(100000, target_density);
  // IndependentMHSimulatorCreator creator(10000, target_density);
  // RandomWalkMHSimulatorCreator creator(10000, target_density);
  ClientCode(creator);
}
