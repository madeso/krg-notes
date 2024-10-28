#include <cmath>

// expand with ideas from:
// https://en.wikipedia.org/wiki/Adaptive_music
// https://www.reddit.com/r/gamedev/comments/4bwnzm/how_to_do_adaptivedynamic_music_in_games/

// convert linear (enerrgy) to decibel and vice versa
// http://www.takuichi.net/hobby/edu/em/db/index.html
float db_from_linear(float linear)
{
    return 10 * std::log10(linear);
}
float linear_from_db(float db)
{
    return std::pow(10, db / 10);
}

float volume_in_db_for_3_tracks_intensity(int i, float volume, float intensity)
{
    // todo(Gustav): expand to support more/fewer tracks?
    const auto factor = std::max((0.5f - std::abs(i / 2.0f - intensity)) / 0.5f, 0.0f);
    const auto vl = linear_from_db(volume) * factor;
    const auto v = db_from_linear(vl);
    return v;
}

// figure out other ways to do dynamic music