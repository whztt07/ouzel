// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#include "SoundDS.h"
#include "AudioDS.h"
#include "audio/SoundData.h"
#include "core/Engine.h"
#include "utils/Log.h"

namespace ouzel
{
    namespace audio
    {
        SoundDS::SoundDS()
        {
        }

        SoundDS::~SoundDS()
        {
            if (buffer3D) buffer3D->Release();
            if (buffer) buffer->Release();
        }

        bool SoundDS::update()
        {
            std::lock_guard<std::mutex> lock(uploadMutex);

            if (dirty & DIRTY_SOUND_DATA)
            {
                if (soundData)
                {
                    if (buffer3D)
                    {
                        buffer3D->Release();
                        buffer3D = nullptr;
                    }

                    if (buffer)
                    {
                        buffer->Release();
                        buffer = nullptr;
                    }

                    IDirectSoundBuffer* tempBuffer = nullptr;

                    WAVEFORMATEX waveFormat;
                    waveFormat.wFormatTag = soundData->getFormatTag();
                    waveFormat.nChannels = soundData->getChannels();
                    waveFormat.nSamplesPerSec = soundData->getSamplesPerSecond();
                    waveFormat.wBitsPerSample = 16;
                    waveFormat.nBlockAlign = waveFormat.nChannels * (waveFormat.wBitsPerSample / 8);
                    waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
                    waveFormat.cbSize = 0;

                    const std::vector<uint8_t> data = soundData->getData();

                    DSBUFFERDESC bufferDesc;
                    bufferDesc.dwSize = sizeof(bufferDesc);
                    bufferDesc.dwFlags = DSBCAPS_CTRLVOLUME;

                    if (soundData->getChannels() < 2)
                    {
                        bufferDesc.dwFlags |= DSBCAPS_CTRL3D;
                    }

                    bufferDesc.dwBufferBytes = static_cast<DWORD>(data.size());
                    bufferDesc.dwReserved = 0;
                    bufferDesc.lpwfxFormat = &waveFormat;
                    bufferDesc.guid3DAlgorithm = GUID_NULL;

                    AudioDS* audioDS = static_cast<AudioDS*>(sharedEngine->getAudio());
                    HRESULT hr = audioDS->getDirectSound()->CreateSoundBuffer(&bufferDesc, &tempBuffer, nullptr);
                    if (FAILED(hr))
                    {
                        Log(Log::Level::ERR) << "Failed to create DirectSound buffer, error: " << hr;
                        return false;
                    }

                    hr = tempBuffer->QueryInterface(IID_IDirectSoundBuffer8, reinterpret_cast<void**>(&buffer));
                    if (FAILED(hr))
                    {
                        Log(Log::Level::ERR) << "Failed to create DirectSound buffer, error: " << hr;
                        tempBuffer->Release();
                        return false;
                    }

                    tempBuffer->Release();

                    uint8_t* bufferPointer;
                    DWORD bufferSize;
                    hr = buffer->Lock(0, bufferDesc.dwBufferBytes, reinterpret_cast<void**>(&bufferPointer), &bufferSize, nullptr, 0, 0);
                    if (FAILED(hr))
                    {
                        Log(Log::Level::ERR) << "Failed to lock DirectSound buffer, error: " << hr;
                        return false;
                    }

                    std::copy(data.begin(), data.end(), bufferPointer);

                    hr = buffer->Unlock(bufferPointer, bufferSize, nullptr, 0);
                    if (FAILED(hr))
                    {
                        Log(Log::Level::ERR) << "Failed to unlock DirectSound buffer, error: " << hr;
                        return false;
                    }

                    if (bufferDesc.dwFlags & DSBCAPS_CTRL3D)
                    {
                        hr = buffer->QueryInterface(IID_IDirectSound3DBuffer8, reinterpret_cast<void**>(&buffer3D));
                        if (FAILED(hr))
                        {
                            Log(Log::Level::ERR) << "Failed to get DirectSound 3D buffer, error: " << hr;
                            return false;
                        }
                    }
                }
            }

            if (dirty & DIRTY_POSITION)
            {
                if (buffer3D)
                {
                    HRESULT hr = buffer3D->SetPosition(position.v[0], position.v[1], position.v[2], DS3D_IMMEDIATE);
                    if (FAILED(hr))
                    {
                        Log(Log::Level::ERR) << "Failed to set DirectSound buffer position, error: " << hr;
                        return false;
                    }
                }
            }

            if (dirty & DIRTY_PLAY_STATE)
            {
                if (buffer)
                {
                    if (shouldPlay)
                    {
                        if (reset)
                        {
                            HRESULT hr = buffer->SetCurrentPosition(0);
                            if (FAILED(hr))
                            {
                                Log(Log::Level::ERR) << "Failed to set DirectSound buffer current position, error: " << hr;
                                return false;
                            }
                        }

                        HRESULT hr = buffer->Play(0, 0, 0);
                        if (FAILED(hr))
                        {
                            Log(Log::Level::ERR) << "Failed to play DirectSound buffer, error: " << hr;
                            return false;
                        }
                    }
                    else
                    {
                        HRESULT hr = buffer->Stop();
                        if (FAILED(hr))
                        {
                            Log(Log::Level::ERR) << "Failed to stop DirectSound buffer, error: " << hr;
                            return false;
                        }

                        hr = buffer->SetCurrentPosition(0);
                        if (FAILED(hr))
                        {
                            Log(Log::Level::ERR) << "Failed to set DirectSound buffer current position, error: " << hr;
                            return false;
                        }
                    }
                }
            }

            dirty = 0;

            return true;
        }
    } // namespace audio
} // namespace ouzel