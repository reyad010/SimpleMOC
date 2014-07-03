#include"SimpleMOC_header.h"

void generate_normal_random_points(double mean, double sigma, int n_pts)
{
	// generate output file
	FILE * out;
	out = fopen("gen_points.txt","w");
	fprintf(out, "Random numbers generated for a normal distribution\n");
	fprintf(out, "Mean = %f\n", mean);
	fprintf(out, "Standard deviation = %f\n", sigma);
	fprintf(out, "Generated points:\n");

	// generate gaussian random points
	for(int i = 0; i < n_pts; i++)
	{
		double random = nrand(mean,sigma);	
		fprintf(out, "%f\n", random);
	}
	// close stream
	fclose(out);

	return;
}
