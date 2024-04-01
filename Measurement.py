class Measurement:
    def __init__(self, temperature, deviceId, adcReading, isSignaling):
        self.temperature = temperature
        self.deviceId = deviceId
        self.adcReading = adcReading
        self.isSignaling = isSignaling