import math
import bisect
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from typing import NamedTuple

class LogEntry(NamedTuple):
    time: int
    mode: int

# TODO:Create a enum or a dictionary to link modes-keynumbers-colors.

class Log:

    def __init__(self):
        self.cycles = 0
        self.active_cycles = 0
        self.off_cycles = 0
        self.ints = 0
        self.lost_ints = 0
        self.step = 0
        self.threhsold = 0
        self.time = 0
        self.mode = 0
        self.worst_save = None
        self.entries = []
        self.v_trace = []
        self.times = []
        self.voltages = []

    def readVoltageTrace(self, path):
        f = open(path, "r")
        for line in f:
            self.v_trace.append(float(line))
        f.close()
        self.generateArrays()

    def generateArrays(self):
        steps = math.floor(self.cycles/self.step)

        # Reconstruct the voltage applied at each time step.
        v = 0
        for i in range(steps+1):
            self.times.append(i*self.step)
            self.voltages.append(self.v_trace[v])
            v = v + 1
            if v >= len(self.v_trace):
                v = 0

        # Add information of the last simulation cycle.
        self.times.append(self.cycles)
        self.voltages.append(self.v_trace[v-1])

        # Apply entries[] to times and voltages so we will
        # know when events happened in simulation.
        for entry in self.entries:
            if entry.time not in self.times:
                bisect.insort(self.times, entry.time)
                ind = bisect.bisect_left(self.times, entry.time)
                self.voltages.insert(ind, self.voltages[ind-1])

    def getModeColor(self, mode):
        if mode == 0:
            return 'green'
        elif mode == 1:
            return 'orange'
        elif mode == 2:
            return 'red'

    def createExecutionPlot(self, path):
        plt.plot(self.times, self.voltages)
        plt.xlim(self.times[0], self.times[-1])
        plt.axhline(y=self.threshold, color='black', linestyle='--')

        prev_entry = LogEntry(self.times[0], 0)
        start = 0
        end = 0
        for entry in self.entries:
            x_interval = [prev_entry.time, entry.time]
            start = bisect.bisect_left(self.times, prev_entry.time)
            end = bisect.bisect_left(self.times, entry.time)
            color = self.getModeColor(prev_entry.mode)
            plt.fill_between(self.times[start:end+1], 0, self.voltages[start:end+1], color=color)
            prev_entry = entry

        plt.fill_between(self.times[end:], 0, self.voltages[end:], color='green')
        plt.xlabel("Cycles")
        plt.ylabel("Voltage (V)")

        on_patch = mpatches.Patch(color=self.getModeColor(0), label='On')
        save_patch = mpatches.Patch(color=self.getModeColor(1), label='Pending interrupt')
        red_patch = mpatches.Patch(color=self.getModeColor(2), label='Off')
        plt.legend(handles=[on_patch, save_patch, red_patch])

        if path != None:
            plt.savefig(path+"execution.pdf")
        else:
            plt.show()

    def findWorstSave(self):
        prev_entry = self.entries[0]
        last_save = None
        worst_save = None
        for entry in self.entries[1:]:
            if entry.mode == 1:
                last_save = entry
            if entry.mode == 2:
                save_time = entry.time - last_save.time
                if worst_save == None:
                    worst_save = save_time
                elif save_time < worst_save:
                    worst_save = save_time
        self.worst_save = worst_save
        return worst_save

    # This function generates results which can be implied by other
    # values in log.
    def generateResults(self):
        active_states = [0, 1]
        off_states = [2]
        if len(self.entries) > 1:
            # Find active cycles
            diff_cycles = 0
            prev_entry = self.entries[0]
            for entry in self.entries[1:]:
                if prev_entry.mode not in active_states:
                    prev_entry = entry
                if entry.mode not in active_states:
                    self.active_cycles = self.active_cycles + entry.time - prev_entry.time
                    prev_entry = entry
            self.active_cycles = self.active_cycles + self.cycles - prev_entry.time
        else:
            self.active_cycles = self.cycles

    def print(self, file):
        print("Cycles:", self.cycles, file=file)
        print("Active cycles:", self.active_cycles, file=file)
        print("Step:", self.step, "cycles", file=file)
        print("Ints:", self.ints, file=file)
        print("Lost ints:", self.lost_ints, file=file)
        print("Threshold:", self.threshold, "V", file=file)
        print("Worst save:", self.worst_save, "cycles", file=file)
        for entry in self.entries:
            print("Time:", entry.time, " Mode:", entry.mode, file=file)

    def setCycles(self, val):
        self.cycles = val

    def setInts(self, val):
        self.ints = val

    def setLostInts(self, val):
        self.lost_ints = val

    def setStep(self, val):
        self.step = val

    def setThreshold(self, val):
        self.threshold = val

    def setTime(self, val):
        self.time = val

    def setMode(self, val):
        self.mode = val

    def addEntry(self):
        entry = LogEntry(self.time, self.mode)
        self.entries.append(entry)

    def setSystem(self, sys):
        self.system = sys