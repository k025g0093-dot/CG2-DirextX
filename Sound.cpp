#include "Sound.h"


Sound::Sound() {

    HRESULT hr = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);

    // 2. 出力先（マスターボイス）生成
    hr = xAudio2->CreateMasteringVoice(&masterVoice);

    if (SUCCEEDED(hr)) {
        isInitialized = true;
    }

}
Sound::~Sound() {
    xAudio2.Reset();
};
SoundData Sound::SoundLoadWave(
    const char* filename) 
{

    //ファイルオープン処理

    std::ifstream file;
    file.open(filename, std::ios_base::binary);
    assert(file.is_open());

    //データ読み込み
    RiffHander riff;
    file.read((char*)&riff, sizeof(riff));

    //リーフかの確認
    if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
        assert(0);
    }
    //WAVEかの確認処理
    if (strncmp(riff.type, "WAVE", 4) != 0) {
        assert(0);
    }

    //フォーマット読み込み
    FormatChunk format = {};
    file.read((char*)&format, sizeof(ChunkHeader));
    if (strncmp(format.chunk.id, "fmt ", 4) != 0) {
        assert(0);
    }
    assert(format.chunk.size <= sizeof(format.fmt));
    file.read((char*)&format.fmt, format.chunk.size);

    ChunkHeader data;
    file.read((char*)&data, sizeof(data));

    //JUNKチャンクを検出した場合
    if (strncmp(data.id, "JUNK", 4) == 0) {
        //読み取り位置をJUNKチャンクの終わりまですすめる
        file.seekg(data.size, std::ios_base::cur);
        //再読み込み
        file.read((char*)&data, sizeof(data));
    }

    if (strncmp(data.id, "data", 4) != 0) {
        assert(0);
    }
    //データのチャンク部分の読み込み
    char* pBuffer = new char[data.size];
    file.read(pBuffer, data.size);
    //WAVEファイルを閉じる
    file.close();

    SoundData soundData = {};

    soundData.wfex = format.fmt;
    soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
    soundData.bufferSize = data.size;

    return soundData;

}

void Sound::SoundPlayWave(

    const SoundData& soundData) 
{

    HRESULT result;

    IXAudio2SourceVoice* pSourceVoice = nullptr;
    result = xAudio2->CreateSourceVoice(
        &pSourceVoice, &soundData.wfex
    );
    assert(SUCCEEDED(result));

    XAUDIO2_BUFFER buf{};
    buf.pAudioData = soundData.pBuffer;
    buf.AudioBytes = soundData.bufferSize;
    buf.Flags = XAUDIO2_END_OF_STREAM;

    result = pSourceVoice->SubmitSourceBuffer(&buf);
    result = pSourceVoice->Start();
}

void Sound::SoundUnLoad(
    SoundData* soundData)
{

    delete[]soundData->pBuffer;
    soundData->pBuffer = 0;
    soundData->bufferSize = 0;
    soundData->wfex={};

}
