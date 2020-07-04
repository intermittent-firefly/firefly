from src.log import *

class Parser:

    def __init__(self, log):
        self.log = log

    def readLog(self, path):
        f = open(path, "r")
        for line in f:
            words = line.strip()
            entry_type = words[1:].partition(">")[0]
            method_name = self.parseSwitch(entry_type)
            method = getattr(self, method_name, lambda: 'Invalid')
            method(line)
        f.close()
        # TODO: not finding the worst save propperly
        #self.log.findWorstSave()
        self.log.generateResults()
        return self.log

    def parseSwitch(self, entry_type):
        return {
            "cycles": "parseCycles",
            "ints": "parseInts",
            "lost_ints": "parseLostInts",
            "time": "parseTime",
            "mode": "parseMode",
            "step": "parseStep",
            "threshold": "parseThreshold",
            "entry": "skipLine",
            "/entry": "parseEntry"
        }.get(entry_type, "parseError")

    def parseSimpleValue(self, line):
        start = line.find('>')
        end = line.find('</')
        val = line[start+1:end]
        return val

    def parseCycles(self, line):
        val = self.parseSimpleValue(line)
        self.log.setCycles(int(val))

    def parseInts(self, line):
        val = self.parseSimpleValue(line)
        self.log.setInts(int(val))

    def parseLostInts(self, line):
        val = self.parseSimpleValue(line)
        self.log.setLostInts(int(val))

    def parseTime(self, line):
        val = self.parseSimpleValue(line)
        self.log.setTime(int(val))

    def parseMode(self, line):
        val = self.parseSimpleValue(line)
        self.log.setMode(int(val))

    def parseStep(self, line):
        val = self.parseSimpleValue(line)
        self.log.setStep(int(val))

    def parseThreshold(self, line):
        val = self.parseSimpleValue(line)
        self.log.setThreshold(float(val))

    def parseEntry(self, line):
        self.log.addEntry()

    def skipLine(self, line):
        pass

    def parseError(self, line):
        print("Cannot parse line: \"", line, "\"")