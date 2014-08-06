#include "anl.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <Windows.h>
#include <string>
using namespace anl;

double map[10000][10000];

double map_file(double map_x_size, double map_y_size) {

	CImplicitGradient ground_gradient;
	ground_gradient.setGradient(0, 0, 0, 1);

#pragma region lowlands
	CImplicitFractal lowland_shape_fractal(BILLOW, GRADIENT, QUINTIC);
	lowland_shape_fractal.setNumOctaves(2);
	lowland_shape_fractal.setFrequency(0.25);
	CImplicitAutoCorrect lowland_autocorrect(0,1);
	lowland_autocorrect.setSource(&lowland_shape_fractal);
	CImplicitScaleOffset lowland_scale(0.125, -0.45);
	lowland_scale.setSource(&lowland_autocorrect);
	CImplicitScaleDomain lowland_y_scale;
	lowland_y_scale.setYScale(0.0);
	lowland_y_scale.setSource(&lowland_scale);
	CImplicitTranslateDomain lowland_terrain;
	lowland_terrain.setSource(&ground_gradient);
	lowland_terrain.setYAxisSource(&lowland_y_scale);
#pragma endregion

#pragma region highlands
	CImplicitFractal highland_shape_fractal(FBM, GRADIENT, QUINTIC);
	highland_shape_fractal.setNumOctaves(4);
	highland_shape_fractal.setFrequency(2);
	//highland_shape_fractal.setSeed(int(highlands_i[5]));
	CImplicitAutoCorrect highland_autocorrect(-1, 1);
	highland_autocorrect.setSource(&highland_shape_fractal);
	CImplicitScaleOffset highland_scale(0.125, 0);
	highland_scale.setSource(&highland_autocorrect);
	CImplicitScaleDomain highland_y_scale;
	highland_y_scale.setYScale(0.0);
	highland_y_scale.setSource(&highland_scale);
	CImplicitTranslateDomain highland_terrain;
	highland_terrain.setSource(&ground_gradient);
	highland_terrain.setYAxisSource(&highland_y_scale);
#pragma endregion

#pragma region mountains
	CImplicitFractal mountain_shape_fractal(RIDGEDMULTI, GRADIENT, QUINTIC);
	mountain_shape_fractal.setNumOctaves(8);
	mountain_shape_fractal.setFrequency(1);
	//mountain_shape_fractal.setSeed(int(mountains_i[5]));
	CImplicitAutoCorrect mountain_autocorrect(-1, 1);
	mountain_autocorrect.setSource(&mountain_shape_fractal);
	CImplicitScaleOffset mountain_scale(0.45, 0.15);
	mountain_scale.setSource(&mountain_autocorrect);
	CImplicitScaleDomain mountain_y_scale;
	mountain_y_scale.setYScale(0.0);
	mountain_y_scale.setSource(&mountain_scale);
	CImplicitTranslateDomain mountain_terrain;
	mountain_terrain.setSource(&ground_gradient);
	mountain_terrain.setYAxisSource(&mountain_y_scale);
#pragma endregion

#pragma region terrain
	CImplicitFractal terrain_type_fractal(FBM, GRADIENT, QUINTIC);
	terrain_type_fractal.setNumOctaves(3);
	terrain_type_fractal.setFrequency(0.125);
	//terrain_type_fractal.setSeed(int(terrain_i[5]));
	CImplicitAutoCorrect terrain_autocorrect(0, 1);
	terrain_autocorrect.setSource(&terrain_type_fractal);
	CImplicitScaleDomain terrain_type_y_scale;
	terrain_type_y_scale.setYScale(0.0);
	terrain_type_y_scale.setSource(&terrain_autocorrect);
	CImplicitCache terrain_type_cache;
	terrain_type_cache.setSource(&terrain_type_y_scale);
	CImplicitSelect highland_mountain_select;
	highland_mountain_select.setLowSource(&highland_terrain);
	highland_mountain_select.setHighSource(&mountain_terrain);
	highland_mountain_select.setControlSource(&terrain_type_cache);
	highland_mountain_select.setThreshold(0.55);
	highland_mountain_select.setFalloff(0.2);
	CImplicitSelect highland_lowland_select;
	highland_lowland_select.setLowSource(&lowland_terrain);
	highland_lowland_select.setHighSource(&highland_mountain_select);
	highland_lowland_select.setControlSource(&terrain_type_cache);
	highland_lowland_select.setThreshold(0.25);
	highland_lowland_select.setFalloff(0.15);
	CImplicitCache highland_lowland_select_cache;
	highland_lowland_select_cache.setSource(&highland_lowland_select);
	CImplicitSelect ground_select;
	ground_select.setLowSource(0.0);
	ground_select.setHighSource(1.0);
	ground_select.setThreshold(0.5);
	ground_select.setControlSource(&highland_lowland_select_cache);
#pragma endregion

#pragma region caves
	CImplicitFractal cave_shape(RIDGEDMULTI, GRADIENT, QUINTIC);
	cave_shape.setNumOctaves(1);
	cave_shape.setFrequency(4);
	//cave_shape.setSeed(int(caveshape_i[5]));
	CImplicitBias cave_attenuate_bias(0.45);
	cave_attenuate_bias.setSource(&highland_lowland_select_cache);
	CImplicitCombiner cave_shape_attenuate(MULT);
	cave_shape_attenuate.setSource(0, &cave_shape);
	cave_shape_attenuate.setSource(1, &cave_attenuate_bias);

	CImplicitFractal cave_preturb_fractal(FBM, GRADIENT, QUINTIC);
	cave_preturb_fractal.setNumOctaves(6);
	cave_preturb_fractal.setFrequency(3);
	//cave_preturb_fractal.setSeed(int(cavepreturb_i[5]));
	CImplicitScaleOffset cave_preturb_scale(0.5, 0);
	cave_preturb_scale.setSource(&cave_preturb_fractal);
	CImplicitTranslateDomain cave_preturb;
	cave_preturb.setSource(&cave_shape_attenuate);
	cave_preturb.setXAxisSource(&cave_preturb_scale);
	CImplicitSelect cave_select;
	cave_select.setLowSource(1.0);
	cave_select.setHighSource(0.0);
	cave_select.setThreshold(0.48);
	cave_select.setControlSource(&cave_preturb);
#pragma endregion

	CImplicitCombiner ground_cave_multiply(MULT);
	ground_cave_multiply.setSource(0, &cave_select);
	ground_cave_multiply.setSource(1, &ground_select);

	CRGBACompositeChannels composite1;
	composite1.setRedSource(&ground_cave_multiply);
	composite1.setGreenSource(&ground_cave_multiply);
	composite1.setBlueSource(&ground_cave_multiply);
	composite1.setAlphaSource(1.0);

	SMappingRanges ranges;

	TArray2D<double> file(map_x_size, map_y_size);
	TArray2D<SRGBA> img(map_x_size, map_y_size);

	std::string filename = "chuck.png";

	map2D(SEAMLESS_NONE, file, ground_cave_multiply, ranges, 0);
	saveDoubleArray("map.tga", &file);

	mapRGBA2D(SEAMLESS_NONE, img, composite1, ranges, 0);
	savePNG(filename, &img);

	return 2;
}

double map_file2(double lowlands_i[11], double highlands_i[11], double mountains_i[11], double terrain_i[14], double caveshape_i[8], double cavepreturb_i[12], double map_x_size, double map_y_size, double seamless) {

	CImplicitGradient ground_gradient;
	ground_gradient.setGradient(0, 0, 0, 1);

#pragma region lowlands

	CImplicitFractal lowland_shape_fractal(int(lowlands_i[0]), int(lowlands_i[1]), lowlands_i[2]);
	lowland_shape_fractal.setNumOctaves(int(lowlands_i[3]));
	lowland_shape_fractal.setFrequency(lowlands_i[4]);
	//lowland_shape_fractal.setSeed(lowlands_i[5]);
	CImplicitAutoCorrect lowland_autocorrect(lowlands_i[6], lowlands_i[7]);
	lowland_autocorrect.setSource(&lowland_shape_fractal);
	CImplicitScaleOffset lowland_scale(lowlands_i[8], lowlands_i[9]);
	lowland_scale.setSource(&lowland_autocorrect);
	CImplicitScaleDomain lowland_y_scale(NULL, lowlands_i[10]);
	lowland_y_scale.setSource(&lowland_scale);
	CImplicitTranslateDomain lowland_terrain;
	lowland_terrain.setSource(&ground_gradient);
	lowland_terrain.setYAxisSource(&lowland_y_scale);
#pragma endregion

#pragma region highlands
	int u;
	int v;
	int w;
	u = int(highlands_i[0]);
	v = int(highlands_i[1]);
	w = int(highlands_i[2]);
	CImplicitFractal highland_shape_fractal(u, v, w);
	highland_shape_fractal.setNumOctaves(int(highlands_i[3]));
	highland_shape_fractal.setFrequency(highlands_i[4]);
	//highland_shape_fractal.setSeed(int(highlands_i[5]));
	CImplicitAutoCorrect highland_autocorrect(highlands_i[6], highlands_i[7]);
	highland_autocorrect.setSource(&highland_shape_fractal);
	CImplicitScaleOffset highland_scale(highlands_i[8], highlands_i[9]);
	highland_scale.setSource(&highland_autocorrect);
	CImplicitScaleDomain highland_y_scale(NULL, highlands_i[10]);
	highland_y_scale.setSource(&highland_scale);
	CImplicitTranslateDomain highland_terrain;
	highland_terrain.setSource(&ground_gradient);
	highland_terrain.setYAxisSource(&highland_y_scale);
#pragma endregion

#pragma region mountains
	int r;
	int s;
	int t;
	r = int(mountains_i[0]);
	s = int(mountains_i[1]);
	t = int(mountains_i[2]);
	CImplicitFractal mountain_shape_fractal(r, s, t);
	mountain_shape_fractal.setNumOctaves(int(mountains_i[3]));
	mountain_shape_fractal.setFrequency(mountains_i[4]);
	//mountain_shape_fractal.setSeed(int(mountains_i[5]));
	CImplicitAutoCorrect mountain_autocorrect(mountains_i[6], mountains_i[7]);
	mountain_autocorrect.setSource(&mountain_shape_fractal);
	CImplicitScaleOffset mountain_scale(mountains_i[8], mountains_i[9]);
	mountain_scale.setSource(&mountain_autocorrect);
	CImplicitScaleDomain mountain_y_scale(NULL, mountains_i[10]);
	mountain_y_scale.setSource(&mountain_scale);
	CImplicitTranslateDomain mountain_terrain;
	mountain_terrain.setSource(&ground_gradient);
	mountain_terrain.setYAxisSource(&mountain_y_scale);
#pragma endregion

#pragma region terrain
	int o;
	int p;
	int q;
	o = int(terrain_i[0]);
	p = int(terrain_i[1]);
	q = int(terrain_i[2]);
	CImplicitFractal terrain_type_fractal(o, p, q);
	terrain_type_fractal.setNumOctaves(int(terrain_i[3]));
	terrain_type_fractal.setFrequency(terrain_i[4]);
	//terrain_type_fractal.setSeed(int(terrain_i[5]));
	CImplicitAutoCorrect terrain_autocorrect(terrain_i[6], terrain_i[7]);
	terrain_autocorrect.setSource(&terrain_type_fractal);
	CImplicitScaleDomain terrain_type_y_scale(NULL, terrain_i[8]);
	CImplicitCache terrain_type_cache;
	terrain_type_cache.setSource(&terrain_type_y_scale);
	CImplicitSelect highland_mountain_select;
	highland_mountain_select.setControlSource(&terrain_type_cache);
	highland_mountain_select.setLowSource(&highland_terrain);
	highland_mountain_select.setHighSource(&mountain_terrain);
	highland_mountain_select.setThreshold(terrain_i[9]);
	highland_mountain_select.setFalloff(terrain_i[10]);
	CImplicitSelect highland_lowland_select;
	highland_lowland_select.setControlSource(&terrain_type_cache);
	highland_lowland_select.setLowSource(&lowland_terrain);
	highland_lowland_select.setHighSource(&highland_mountain_select);
	highland_lowland_select.setThreshold(terrain_i[11]);
	highland_lowland_select.setFalloff(terrain_i[12]);
	CImplicitCache highland_lowland_select_cache;
	highland_lowland_select_cache.setSource(&highland_lowland_select);
	CImplicitSelect ground_select;
	ground_select.setControlSource(&highland_lowland_select_cache);
	ground_select.setHighSource(1);
	ground_select.setThreshold(terrain_i[13]);
#pragma endregion

#pragma region caves
	int l;
	int m;
	int n;
	l = int(caveshape_i[0]);
	m = int(caveshape_i[1]);
	n = int(caveshape_i[2]);
	CImplicitFractal cave_shape(l, m, n);
	cave_shape.setNumOctaves(int(caveshape_i[3]));
	cave_shape.setFrequency(caveshape_i[4]);
	//cave_shape.setSeed(int(caveshape_i[5]));
	CImplicitBias cave_attenuate_bias(caveshape_i[6]);
	cave_attenuate_bias.setSource(&highland_lowland_select_cache);
	CImplicitCombiner cave_shape_attenuate(caveshape_i[7]);
	cave_shape_attenuate.setSource(0, &cave_shape);
	cave_shape_attenuate.setSource(1, &cave_attenuate_bias);
	int i;
	int j;
	int k;	
	i = int(cavepreturb_i[0]);
	j = int(cavepreturb_i[1]);
	k = int(cavepreturb_i[2]);
	CImplicitFractal cave_preturb_fractal(i, j, k);
	cave_preturb_fractal.setNumOctaves(int(cavepreturb_i[3]));
	cave_preturb_fractal.setFrequency(cavepreturb_i[4]);
	//cave_preturb_fractal.setSeed(int(cavepreturb_i[5]));
	CImplicitScaleOffset cave_preturb_scale(cavepreturb_i[6], cavepreturb_i[7]);
	cave_preturb_scale.setSource(&cave_preturb_fractal);
	CImplicitTranslateDomain cave_preturb;
	cave_preturb.setSource(&cave_shape_attenuate);
	cave_preturb.setYAxisSource(&cave_preturb_scale);
	CImplicitSelect cave_select;
	cave_select.setControlSource(&cave_preturb);
	cave_select.setLowSource(cavepreturb_i[8]);
	cave_select.setHighSource(cavepreturb_i[9]);
	cave_select.setThreshold(cavepreturb_i[10]);
	cave_select.setFalloff(cavepreturb_i[11]);
#pragma endregion

	CImplicitCombiner ground_cave_multiply(MULT);
	ground_cave_multiply.setSource(0, &cave_select);
	ground_cave_multiply.setSource(1, &ground_select);

	CRGBACompositeChannels composite1;
	composite1.setRedSource(&ground_cave_multiply);
	composite1.setGreenSource(&ground_cave_multiply);
	composite1.setBlueSource(&ground_cave_multiply);
	composite1.setAlphaSource(1.0);

	SMappingRanges ranges;
	
	TArray2D<double> file(map_x_size, map_y_size);
	TArray2D<SRGBA> img(map_x_size, map_y_size);

	std::string filename = "chuck2.png";
	
	map2D(seamless, file, ground_cave_multiply, ranges, 0);
	saveDoubleArray("map2.tga", &file);

	mapRGBA2D(seamless, img, composite1, ranges, 0);
	savePNG(filename, &img);

	return 2;
}

double map_test(double map_x_size, double map_y_size) {
	anl::CMWC4096 rnd;
	rnd.setSeedTime();

	CImplicitGradient ground_gradient;
	ground_gradient.setGradient(0, 0, 0, 1);

	CImplicitFractal ground_shape_fractal(BILLOW, GRADIENT, QUINTIC);
	ground_shape_fractal.setNumOctaves(2);
	ground_shape_fractal.setFrequency(1);
	ground_shape_fractal.setSeed(rnd.get());


	CImplicitScaleOffset ground_scale(0.5, 0);
	ground_scale.setSource(&ground_shape_fractal);

	CImplicitScaleDomain ground_scale_y(NULL, 0);
	ground_scale_y.setSource(&ground_scale);

	CImplicitTranslateDomain ground_preturb;
	ground_preturb.setSource(&ground_gradient);
	ground_preturb.setYAxisSource(&ground_scale_y);

	CImplicitSelect ground_select;
	ground_select.setControlSource(&ground_preturb);

	ground_select.setHighSource(1);
	ground_select.setThreshold(0.5);

	CRGBACompositeChannels compose1;
	compose1.setRedSource(&ground_gradient);
	compose1.setGreenSource(&ground_gradient);
	compose1.setBlueSource(&ground_gradient);
	compose1.setAlphaSource(1.0);


	CRGBACompositeChannels compose2;
	compose2.setRedSource(&ground_shape_fractal);
	compose2.setGreenSource(&ground_shape_fractal);
	compose2.setBlueSource(&ground_shape_fractal);
	compose2.setAlphaSource(1.0);

	CRGBACompositeChannels compose3;
	compose3.setRedSource(&ground_scale);
	compose3.setGreenSource(&ground_scale);
	compose3.setBlueSource(&ground_scale);
	compose3.setAlphaSource(1.0);

	CRGBACompositeChannels compose4;
	compose4.setRedSource(&ground_scale_y);
	compose4.setGreenSource(&ground_scale_y);
	compose4.setBlueSource(&ground_scale_y);
	compose4.setAlphaSource(1.0);

	CRGBACompositeChannels compose5;
	compose5.setRedSource(&ground_preturb);
	compose5.setGreenSource(&ground_preturb);
	compose5.setBlueSource(&ground_preturb);
	compose5.setAlphaSource(1.0);

	CRGBACompositeChannels compose6;
	compose6.setRedSource(&ground_select);
	compose6.setGreenSource(&ground_select);
	compose6.setBlueSource(&ground_select);
	compose6.setAlphaSource(1.0);

	SMappingRanges ranges;

	TArray2D<double> file(map_x_size, map_y_size);
	TArray2D<SRGBA> img(map_x_size, map_y_size);

	std::string filename = "test2.png";
	std::string filename2 = "test3.png";
	std::string filename3 = "test4.png";
	std::string filename4 = "test5.png";
	std::string filename5 = "test6.png";
	std::string filename6 = "test7.png";

	map2D(0, file, ground_preturb, ranges, 0);
	

	for (int x = 0; x < map_x_size; x++) {
		for (int y = 0; y < map_y_size; y++) {
			map[x][y] = returnDoubleArray(x, y, &file);
		}
	}

	saveDoubleArray("test.tga", &file);
	
	mapRGBA2D(0, img, compose1, ranges, 0);
	savePNG(filename, &img);
	
	mapRGBA2D(0, img, compose2, ranges, 0);
	savePNG(filename2, &img);

	mapRGBA2D(0, img, compose3, ranges, 0);
	savePNG(filename3, &img);

	mapRGBA2D(0, img, compose4, ranges, 0);
	savePNG(filename4, &img);

	mapRGBA2D(0, img, compose5, ranges, 0);
	savePNG(filename5, &img);

	mapRGBA2D(0, img, compose6, ranges, 0);
	savePNG(filename6, &img);

	return 0;
}

double return_map(int x, int y) {
	return map[x][y];
}

double map_test2() {
	anl::CMWC4096 rnd;
	rnd.setSeedTime();
	anl::CImplicitFractal frac1(anl::FBM, anl::GRADIENT, anl::QUINTIC);
	anl::CImplicitFractal frac2(anl::FBM, anl::GRADIENT, anl::QUINTIC);
	anl::CImplicitFractal frac3(anl::FBM, anl::GRADIENT, anl::QUINTIC);
	anl::CImplicitFractal frac4(anl::RIDGEDMULTI, anl::GRADIENT, anl::QUINTIC);
	anl::CImplicitFractal frac5(anl::FBM, anl::GRADIENT, anl::QUINTIC);
	anl::CImplicitFractal frac6(anl::FBM, anl::GRADIENT, anl::QUINTIC);
	anl::CImplicitFractal frac7(anl::FBM, anl::GRADIENT, anl::QUINTIC);

	frac1.setSeed(rnd.get());
	frac2.setSeed(rnd.get());
	frac3.setSeed(rnd.get());
	frac4.setSeed(rnd.get());
	frac5.setSeed(rnd.get());
	frac6.setSeed(rnd.get());
	frac7.setSeed(rnd.get());


	anl::CImplicitAutoCorrect ac1(0.0, 1.0), ac2(0.0, 1.0), ac3(0, 1.0),
		ac4(0.0, 360.0), ac5(-1.0, 1.0), ac6(-1.0, 1.0), ac7(-1.0, 1.0);
	ac1.setSource(&frac1);
	ac2.setSource(&frac2);
	ac3.setSource(&frac3);
	ac4.setSource(&frac4);
	ac5.setSource(&frac5);
	ac6.setSource(&frac6);
	ac7.setSource(&frac7);

	anl::CRGBACompositeChannels compose1(anl::RGB);

	compose1.setRedSource(&ac1);
	compose1.setGreenSource(&ac2);
	compose1.setBlueSource(&ac3);
	compose1.setAlphaSource(1.0);


	anl::CRGBARotateColor rot;
	rot.setAngle(&ac4);
	rot.setAxisX(&ac5);
	rot.setAxisY(&ac6);
	rot.setAxisZ(&ac7);
	rot.setNormalizeAxis(true);
	rot.setSource(&compose1);

	TArray2D<SRGBA> img(256, 256);

	std::string filename1 = "a.png";
	std::string filename2 = "b.png";

	anl::SMappingRanges ranges;
	mapRGBA2D(anl::SEAMLESS_NONE, img, rot, ranges, 0);
	savePNG(filename1, &img);
	mapRGBA2D(anl::SEAMLESS_NONE, img, compose1, ranges, 0);
	int error = savePNG(filename2, &img); 

	if (error == 10) {
		return 10;
	}
	else {
		return 5;
	}
	
}