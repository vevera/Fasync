#include <chrono>
#include <iostream>
#include <random>
#include <string>

#include "callback.hpp"
#include "scheduler.hpp"

void func1(double a, double c);

class Test {
public:
	Test() {
		Fasync::Connect(&sumSignal, this, &Test::sum2);
		Fasync::Connect(&mulSignal, [](int a, int b) {
			int mul = (a * b);
			std::cout << "My Mul was: " << mul << std::endl;
		});
	}

	void sum2(int a, int b) {
		int sum = (a + b);

		std::cout << "My Sum was: " << sum << std::endl;
		Fasync::Emit(&mulSignal, sum, 2);
	}

	Fasync::Signal<int, int> sumSignal;
	Fasync::Signal<int, int> mulSignal;
};


class Test2 {
public:
	Test2(std::string name) : name_{ name } {}

	void doWork(int a, int b) {
		std::cout << "Test2: " << name_ << " result: " << (a * (a + b)) << std::endl;
	}

	void doWorkConst(int a, int b) const{
		std::cout << "Test2 const: " << name_ << " result: " << (a * (a + b)) << std::endl;
	}

	std::string name_;
};


static void doWork(int a, int b) {
	std::cout << "DoWork result: " << (a) << std::endl;
};


int main(void) {

	// Signals example

	{
		std::cout << "Signals Example!!!\n";

		Test t;

		Test2 ta("ta");
		Test2 tb("tb");
		Test2 tc("tc");

		// A callback assigned to a signal must return void
		Fasync::Connect(&t.sumSignal, &ta, &Test2::doWork);
		// A callback argument types must match the signal's definition exactly
		Fasync::Connect(&t.sumSignal, &tb, &Test2::doWorkConst);
		// Connect mulSignal to a global function
		Fasync::Connect(&t.mulSignal, &doWork);

		// Emit a signal with arguments 5 and 2
		Fasync::Emit(&t.sumSignal, 5, 2);
	}

	// Scheduler Example

	{
		std::cout << "Scheduler Example!!!\n";

		std::mt19937_64 gen;
		std::uniform_real_distribution<double> dist(1.0, 300.0);

		size_t n = 10;

		auto begin = std::chrono::steady_clock::now();

		Fasync::Scheduler s;

		for (size_t i = 0; i < n; ++i) {
			s.Add(func1, dist(gen), dist(gen));
		}

		auto end = std::chrono::steady_clock::now();

		std::cout << "time elapsed ts: " << (std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()) << std::endl;

		begin = std::chrono::steady_clock::now();

		{
			for (size_t i = 0; i < n; ++i) {
				func1(dist(gen), dist(gen));
			}
		}

		end = std::chrono::steady_clock::now();

		std::cout << "time elapsed sync: " << (std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()) << std::endl;
	}

	return 0;
}


void func1(double a, double c) {
	if (c == 0) {
		return;
	}

	size_t n = 1000000000;

	auto res = a / c;
	double multiplier = 2.0;

	for (size_t i = 0; i < n; ++i) {
		res = res * multiplier;

		if (i % 2 == 0) multiplier = 0.50000;
		else multiplier = 2.0;
	}

	std::cout << "Number: " << res << "\n";
}