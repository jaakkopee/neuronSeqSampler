import pydub

def convert_wav_to_mp3(wav_file, mp3_file):
    audio = pydub.AudioSegment.from_wav(wav_file)
    audio.export(mp3_file, format="mp3")
# Example usage
# convert_wav_to_mp3("input.wav", "output.mp3")

if __name__ == "__main__":
    import sys
    if len(sys.argv) != 3:
        print("Usage: python convert_wav_to_mp3.py <input.wav> <output.mp3>")
    else:
        convert_wav_to_mp3(sys.argv[1], sys.argv[2])