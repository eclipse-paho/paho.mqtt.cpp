#pragma once
#include <string>
#include <map>

struct ScoreWeights {
    double latency;
    double bandwidth;
    double connection;
};

static const std::map<std::string, ScoreWeights> CATEGORY_WEIGHTS = {
    {"sensor",    {0.6, 0.2, 0.2}},
    {"camera",    {0.2, 0.6, 0.2}},
    {"meter",     {0.6, 0.2, 0.2}},
    {"light",     {0.6, 0.2, 0.2}},
    {"appliance", {0.6, 0.2, 0.2}},
    {"wearable",  {0.3, 0.4, 0.3}},
    {"beacon",    {0.6, 0.2, 0.2}},
    {"traffic",   {0.4, 0.2, 0.4}},
    {"drone",     {0.3, 0.5, 0.2}},
    {"rfid",      {0.3, 0.2, 0.5}},
    {"signage",   {0.2, 0.6, 0.2}},
}; 