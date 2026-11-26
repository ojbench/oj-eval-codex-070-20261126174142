# ifndef _SJTU_CPP_FINAL_CPU_HPP_
# define _SJTU_CPP_FINAL_CPU_HPP_

# include <vector>
# include "task.hpp"

using namespace std;

typedef unsigned int uint;

namespace sjtu {
    // CPU base class, modifications is not allowed.
    class CPU {
    protected:
        CPUState state;
        vector<Task> tasks;

    public:
        CPU() : tasks() {
            state = idle;
        }

        // Add a new task.
        int addTask(const Task &t) {
            tasks.push_back(t);
            return 1;
        }

        // Change the priority of one process, return 1 if success and return 0 if fail.
        int changePriority(uint task_id, uint priority) {
            for (auto &task: tasks)
                if (task.task_id == task_id) {
                    task.priority = priority;
                    return 1;
                }
            return 0;
        }

        virtual pair<CPUState, uint> run() = 0;

        virtual ~ CPU() = default;
    };

    class CPU_FCFS : public CPU {
    public:
        pair<CPUState, uint> run() override {
            for (auto &task : tasks) {
                if (task.time == 0) continue;
                --task.time;
                state = busy;
                return make_pair(state, task.task_id);
            }
            state = idle;
            return make_pair(state, 0u);
        }
    };

    class CPU_SRTF : public CPU {
    public:
        pair<CPUState, uint> run() override {
            Task *selected = nullptr;
            for (auto &task : tasks) {
                if (task.time == 0) continue;
                // Strict comparison preserves arrival order on ties.
                if (selected == nullptr || task.time < selected->time)
                    selected = &task;
            }
            if (selected == nullptr) {
                state = idle;
                return make_pair(state, 0u);
            }
            --selected->time;
            state = busy;
            return make_pair(state, selected->task_id);
        }
    };

    class CPU_PRIORITY : public CPU {
    public:
        pair<CPUState, uint> run() override {
            Task *selected = nullptr;
            for (auto &task : tasks) {
                if (task.time == 0) continue;
                if (selected == nullptr || task.priority < selected->priority)
                    selected = &task;
            }
            if (selected == nullptr) {
                state = idle;
                return make_pair(state, 0u);
            }
            --selected->time;
            state = busy;
            return make_pair(state, selected->task_id);
        }
    };
}

# endif
