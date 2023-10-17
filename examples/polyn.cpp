/***

	polyn.cpp

	Find a zero of a polynomial in up to two variables, using the genetic optimizer
	and the function inverter.

	Copyright (c) 2022 Chris Street.

***/
#define CODEHAPPY_NATIVE
#include <libcodehappy.h>

struct Polyn {
	void fill_from_input();
	double calculate(double* in) const;
	float calculate(float* in) const;

	u32 x_deg;
	u32 y_deg;
	double* coeff;
};

double Polyn::calculate(double* in) const {
	double xx = 1.0, yy;
	double val = 0.;
	u32 c = 0;

	for (u32 dx = 0; dx <= x_deg; ++dx) {
		yy = 1.0;
		for (u32 dy = 0; dy <= y_deg; ++dy) {
			val += coeff[c++] * xx * yy;
			yy *= in[1];
		}
		xx *= in[0];
	}

	return val;
}

float Polyn::calculate(float* in) const {
	float xx = 1.0, yy;
	float val = 0.;
	u32 c = 0;

	for (u32 dx = 0; dx <= x_deg; ++dx) {
		yy = 1.0;
		for (u32 dy = 0; dy <= y_deg; ++dy) {
			val += (float)(coeff[c++]) * xx * yy;
			yy *= in[1];
		}
		xx *= in[0];
	}

	return val;
}

void Polyn::fill_from_input() {
	std::cout << "Enter the highest degree of X in the polynomial: ";
	std::cin >> x_deg;
	std::cout << "Enter the highest degree of Y in the polynomial: ";
	std::cin >> y_deg;
	coeff = new double [(x_deg + 1) * (y_deg + 1)];

	u32 c = 0;
	for (u32 dx = 0; dx <= x_deg; ++dx) {
		for (u32 dy = 0; dy <= y_deg; ++dy) {
			std::cout << "Enter the coefficient for x^" << dx << " y^" << dy << ": ";
			std::cin >> coeff[c++];
		}
	}
}

/* We're minimizing the absolute value of the polynomial. */
double polyn_callback(double* in, void* user_data) {
	const Polyn* poly = (const Polyn *)user_data;
	return fabs(poly->calculate(in));
}

/* Polynomial evaluation for the InvFn. */
bool polyn_invertor(float* in, float* out, void* user_data) {
	const Polyn* poly = (const Polyn *)user_data;
	out[0] = poly->calculate(in);
	return true;
}

int main(void) {
	/* Read polynomial */
	Polyn p;
	p.fill_from_input();

	/* Find a zero with the genetic optimizer. */
	GeneticOptimizer go(2, polyn_callback, (void *)&p);
	go.verbose(false);
	go.set_optimum_is_zero(true);
	std::cout << "*** Genetic optimizer\n";
	double* inp = go.optimize_min(0.00001);
	std::cout << "Zero at x = " << inp[0] << ", y = " << inp[1] << ".\n";
	std::cout << "Polynomial evaluation at this zero: " << p.calculate(inp) << "\n\n";

	/* Train the InvFn model and ask it for a zero. */
	InvFn inv(2, 1, polyn_invertor, (void *)&p);
	float zero[2];
	inv.verbose(false);
	inv.set_radius(5.);
	std::cout << "*** Neural net inversion model\n";
	std::cout << "Training the model...\n";
	inv.train();
	inv.train();
	std::cout << "Function invertor trained; average loss reported: " << inv.avg_loss() << ".\n";
	inv.inv(0.0, zero);
	std::cout << "Zero from model: x = " << zero[0] << ", y = " << zero[1] << ".\n";
	std::cout << "Polynomial evaluation at this zero: " << p.calculate(zero) << "\n\n";

	return 0;
}

/* end polyn.cpp */