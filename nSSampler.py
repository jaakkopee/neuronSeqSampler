from pydub import AudioSegment
from pydub.playback import play
from threading import Thread

class Sample:
    def __init__(self, file_path):
        self.file_path = file_path
        self.audio = AudioSegment.from_file(file_path)

    def play(self):
        # Play the audio in a separate thread to avoid blocking
        Thread(target=self._play_audio).start()

    def _play_audio(self):
        play(self.audio)

class SampleKit:
    def __init__(self):
        self.samples = {}

    def add_sample(self, name, file_path):
        self.samples[name] = Sample(file_path)

    def play_sample(self, name):
        if name in self.samples:
            self.samples[name].play()
        else:
            print(f"Sample '{name}' not found.")

            