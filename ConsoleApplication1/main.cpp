#include "accidental_noise.cpp"

void main() {
	double lowlands[11];
	double highlands[11];
	double mountain[11];
	double terrain[14];
	double caveshape[8];
	double cavepreturb[12];

	lowlands[0] = 2;
	lowlands[1] = 1;
	lowlands[2] = 3;
	lowlands[3] = 2;
	lowlands[4] = 0.25;
	lowlands[6] = 0;
	lowlands[7] = 1;
	lowlands[8] = 0.125;
	lowlands[9] = -0.45;
	lowlands[10] = 0;

	highlands[0] = 0;
	highlands[1] = 1;
	highlands[2] = 3;
	highlands[3] = 4;
	highlands[4] = 2;
	highlands[6] = -1;
	highlands[7] = 1;
	highlands[8] = 0.25;
	highlands[9] = 0;
	highlands[10] = 0;

	mountain[0] = 2;
	mountain[1] = 1;
	mountain[2] = 3;
	mountain[3] = 8;
	mountain[4] = 1;
	mountain[6] = -1;
	mountain[7] = 1;
	mountain[8] = 0.45;
	mountain[9] = 0.15;
	mountain[10] = 0.25;

	terrain[0] = 0;
	terrain[1] = 1;
	terrain[2] = 3;
	terrain[3] = 3;
	terrain[4] = 0.125;
	terrain[6] = 0;
	terrain[7] = 1;
	terrain[8] = 0;
	terrain[9] = 0.55;
	terrain[10] = 0.2;
	terrain[11] = 0.25;
	terrain[12] = 0.15;
	terrain[13] = 0.5;

	caveshape[0] = 1;
	caveshape[1] = 1;
	caveshape[2] = 3;
	caveshape[3] = 1;
	caveshape[4] = 4;
	caveshape[6] = 0.45;
	caveshape[7] = 1;

	cavepreturb[0] = 0;
	cavepreturb[1] = 1;
	cavepreturb[2] = 3;
	cavepreturb[3] = 6;
	cavepreturb[4] = 3;
	cavepreturb[6] = 0.5;
	cavepreturb[7] = 0;
	cavepreturb[8] = 1;
	cavepreturb[9] = 0;
	cavepreturb[10] = 0.48;
	cavepreturb[11] = 0;

	map_file();

	//map_file2(lowlands, highlands, mountain, terrain, caveshape, cavepreturb, 800, 200, 0);

	//map_test(700, 300);
	//map_test2();
}