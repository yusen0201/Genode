#include <base/printf.h>

#include <glucose/core/Solver.h>
#include <glucose/core/Dimacs.h>

using namespace Glucose;

int main()
{
	Solver solver;

	gzFile gzfile_components = gzopen("Test.cnf", "rb");	
	if (gzfile_components != NULL) {
		parse_DIMACS(gzfile_components, solver);
		gzclose(gzfile_components);
		PINF("Okay");
	}
	else {
		PERR("Unable to open file \"Test.cnf\"");
		return 1;
	}

	return 0;
}
