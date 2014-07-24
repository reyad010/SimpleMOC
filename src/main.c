#include"SimpleMOC_header.h"

int main( int argc, char * argv[] )
{
	int version = 0;
	int mype = 0;
	int nranks;
	//test

	#ifdef MPI
	MPI_Status stat;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nranks);
	MPI_Comm_rank(MPI_COMM_WORLD, &mype);
	#endif
	
	#ifdef PAPI
	papi_serial_init();
	#endif

	if( mype == 0 )
		logo(version);

	srand(time(NULL) * (mype+1));

	Input input = get_input();
	
	CommGrid grid = init_mpi_grid( input );

	if( mype == 0 )
		print_input_summary(input);

	Params params = build_tracks( input );

	float res;
	float keff = 1.0;
	int num_iters = 1;


	double time_transport = 0;
	double time_flux_exchange = 0;
	double time_renormalize_flux = 0;
	double time_update_sources = 0;
	double time_compute_keff = 0;
	double start, stop;

	if(mype==0)
	{
		center_print("SIMULATION", 79);
		border_print();
	}

	for( int i = 0; i < num_iters; i++)
	{
		// Transport Sweep
		start = get_time();
		transport_sweep(params, input);
		stop = get_time();
		time_transport += stop-start;

		// Domain Boundary Flux Exchange (MPI)
		#ifdef MPI
		start = get_time();
		fast_transfer_boundary_fluxes(params, input, grid);
		stop = get_time();
		time_flux_exchange += stop-start;
		#endif

		// Flux Renormalization
		start = get_time();
		renormalize_flux(params,input, grid);
		stop = get_time();
		time_renormalize_flux += stop-start;

		// Update Source Regions
		start = get_time();
		res = update_sources(params, input, keff);
		stop = get_time();
		time_update_sources += stop-start;

		// Calculate K-Effective
		start = get_time();
		keff = compute_keff(params, input, grid);
		stop = get_time();
		time_compute_keff += stop-start;
		if( mype == 0 )
			printf("keff = %lf\n", keff);
	}

	if( mype == 0 )
	{
		border_print();
		center_print("Results Summary", 79);
		border_print();
		double time_total = time_transport+time_flux_exchange+
			time_renormalize_flux + time_update_sources+
			time_compute_keff;

		printf("Transport Sweep Time:         %6.2lf sec   (%4.1lf%%)\n", time_transport, 100*time_transport/time_total);
		printf("Domain Flux Exchange Time:    %6.2lf sec   (%4.1lf%%)\n", time_flux_exchange, 100*time_flux_exchange/time_total);
		printf("Flux Renormalization Time:    %6.2lf sec   (%4.1lf%%)\n", time_renormalize_flux, 100*time_renormalize_flux/time_total);
		printf("Update Source Time:           %6.2lf sec   (%4.1lf%%)\n", time_update_sources, 100*time_update_sources/time_total);
		printf("K-Effective Calc Time:        %6.2lf sec   (%4.1lf%%)\n", time_compute_keff, 100*time_compute_keff/time_total);
		printf("Total Time:                   %6.2lf sec\n", time_total);
		printf("Tracks per Second per domain: "); fancy_int( input.ntracks / time_transport );

		border_print();
	}

	free_2D_tracks( params.tracks_2D );
	free_tracks( params.tracks );
	
	#ifdef MPI
	MPI_Finalize();
	#endif

	return 0;
}
