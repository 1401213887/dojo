#include "IteratedNoise.h"

using namespace Dojo;

IteratedNoise::IteratedNoise(Noise& noise, const Iteration::List& levels) :
	mBase(noise),
	mLevels(levels) {

	for(auto&& level : levels) {
		mTotalWeight += level.weight;
	}
}

float IteratedNoise::noise(float x, float y) const {
	float f = 0;

	for (auto&& level : mLevels) {
		f += mBase.normalizedNoise(x, y, level.scale) * (level.weight / mTotalWeight);
	}

	return f;
}
