#ifndef LINEARSCAN_HPP
#define LINEARSCAN_HPP

// don't include other headfiles
#include <string>
#include <vector>
#include <set>

class Location {
public:
    // return a string that represents the location
    virtual std::string show() const = 0;
    virtual int getId() const = 0;
};

class Register : public Location {
private:
    int id;
public:
    Register(int regId) : id(regId) {}
    virtual std::string show() const {
        return "reg" + std::to_string(id);
    }
    virtual int getId() const {
        return id;
    }
};

class StackSlot : public Location {
public:
    StackSlot() {}
    virtual std::string show() const {
        return "stack";
    }
    virtual int getId() const {
        return -1;
    }
};

struct LiveInterval {
    int startpoint;
    int endpoint;
    Location* location = nullptr;
};

class LinearScanRegisterAllocator {
private:
    int regNum;
    std::vector<Location*> freeRegs;
    
    struct CompareInterval {
        bool operator()(const LiveInterval* a, const LiveInterval* b) const {
            return a->endpoint < b->endpoint;
        }
    };
    std::set<LiveInterval*, CompareInterval> active;

    void expireOldIntervals(LiveInterval& i) {
        for (auto it = active.begin(); it != active.end(); ) {
            if ((*it)->endpoint >= i.startpoint) {
                break;
            }
            freeRegs.push_back((*it)->location);
            it = active.erase(it);
        }
    }

    void spillAtInterval(LiveInterval& i) {
        LiveInterval* spill = *active.rbegin();
        if (spill->endpoint > i.endpoint) {
            i.location = spill->location;
            spill->location = new StackSlot();
            active.erase(spill);
            active.insert(&i);
        } else {
            i.location = new StackSlot();
        }
    }

public:
    LinearScanRegisterAllocator(int regNum) : regNum(regNum) {
        for (int i = regNum - 1; i >= 0; --i) {
            freeRegs.push_back(new Register(i));
        }
    }

    void linearScanRegisterAllocate(std::vector<LiveInterval>& intervalList) {
        for (auto& i : intervalList) {
            expireOldIntervals(i);
            if (active.size() == (size_t)regNum) {
                spillAtInterval(i);
            } else {
                Location* reg = freeRegs.back();
                freeRegs.pop_back();
                i.location = reg;
                active.insert(&i);
            }
        }
    }
};

#endif
