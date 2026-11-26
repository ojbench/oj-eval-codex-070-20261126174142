#include "src.hpp"
#include <cassert>
#include <limits>
#include <random>
#include <tuple>

void expect(sjtu::CPU &cpu, uint id) {
    const auto result = cpu.run();
    assert(result.first == (id == 0 ? sjtu::idle : sjtu::busy));
    assert(result.second == id);
}

int main() {
    sjtu::CPU_FCFS fcfs;
    expect(fcfs, 0);
    fcfs.addTask(sjtu::Task(3, 10, 3));
    fcfs.addTask(sjtu::Task(2, 0, 1));
    for (uint id : {3u, 3u, 3u, 2u, 0u}) expect(fcfs, id);

    sjtu::CPU_SRTF srtf;
    srtf.addTask(sjtu::Task(9, 0, 3));
    expect(srtf, 9);
    srtf.addTask(sjtu::Task(1, 0, 1));
    expect(srtf, 1);
    srtf.addTask(sjtu::Task(2, 0, 2));
    for (uint id : {9u, 9u, 2u, 2u, 0u}) expect(srtf, id);

    sjtu::CPU_PRIORITY priority;
    priority.addTask(sjtu::Task(1, 10, 3));
    expect(priority, 1);
    priority.addTask(sjtu::Task(2, 9, 2));
    expect(priority, 2);
    assert(priority.changePriority(1, 8) == 1);
    priority.addTask(sjtu::Task(3, 11, 1));
    for (uint id : {1u, 1u, 2u, 3u, 0u}) expect(priority, id);
    assert(priority.changePriority(42, 0) == 0);

    std::mt19937 rng(1448);
    for (int policy = 0; policy < 3; ++policy) {
        for (int trial = 0; trial < 100; ++trial) {
            sjtu::CPU_FCFS a;
            sjtu::CPU_SRTF b;
            sjtu::CPU_PRIORITY c;
            sjtu::CPU *cpu = policy == 0 ? static_cast<sjtu::CPU *>(&a)
                           : policy == 1 ? static_cast<sjtu::CPU *>(&b) : &c;
            std::vector<sjtu::Task> model;
            for (int tick = 0; tick <= 1000; ++tick) {
                if (model.size() < 100 && rng() % 4 == 0) {
                    uint duration = rng() % 15;
                    if (model.empty()) duration = std::numeric_limits<uint>::max();
                    model.emplace_back(1000u - model.size(), rng() % 8, duration);
                    cpu->addTask(model.back());
                }
                if (!model.empty() && rng() % 3 == 0) {
                    auto &task = model[rng() % model.size()];
                    task.priority = rng() % 8;
                    assert(cpu->changePriority(task.task_id, task.priority) == 1);
                }
                std::vector<std::pair<uint, unsigned>> ready;
                for (unsigned i = 0; i < model.size(); ++i) {
                    if (model[i].time == 0) continue;
                    uint key = policy == 0 ? 0 : policy == 1 ? model[i].time : model[i].priority;
                    ready.emplace_back(key, i);
                }
                if (ready.empty()) {
                    expect(*cpu, 0);
                } else {
                    auto best = ready.front();
                    for (const auto &entry : ready) if (entry < best) best = entry;
                    auto &task = model[best.second];
                    expect(*cpu, task.task_id);
                    --task.time;
                }
            }
        }
    }
}
