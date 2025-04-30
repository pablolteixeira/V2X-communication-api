#ifndef REFERENCE_BUFFER_H
#define REFERENCE_BUFFER_H

#include <mutex>
#include <atomic>

template <typename T, size_t SIZE>
class ReferenceBuffer 
{
public:
    ReferenceBuffer() {
        for (size_t i = 0; i < SIZE; i++) {
            _data_buffer[i] = nullptr;
            _ref_counts[i] = 0;
        }
    }  

    ~ReferenceBuffer() {
        std::lock_guard<std::mutex> lock(_mutex);
        for (size_t i = 0; i < SIZE; i++) {
            if (_data_buffer[i] != nullptr && _ref_counts[i] == 0) {
                delete _data_buffer[i];
                _data_buffer[i] = nullptr;
            }
        }
    }

    T* alloc(int ref_count) {
        std::lock_guard<std::mutex> lock(_mutex);

        for (size_t i = 0; i < SIZE; i++) {
            if (_data_buffer[i] == nullptr && _ref_counts[i] == 0) {
                _data_buffer[i] = new T();
                _ref_counts[i] = ref_count;
                
                return _data_buffer[i];
            }
        }

        return nullptr;
    }

    void free(T* data) {
        std::lock_guard<std::mutex> lock(_mutex);

        for (size_t i = 0; i < SIZE; i++) {
            if (_data_buffer[i] == data) {
                _ref_counts[i]--;

                if (_ref_counts[i] <= 0) {
                    delete _data_buffer[i];
                    _data_buffer[i] = nullptr;
                    _ref_counts[i] = 0;
                }
                break;
            }
        }
    }

private:
    T* _data_buffer[SIZE];
    int _ref_counts[SIZE];
    std::mutex _mutex;
};

#endif // REFERENCE_BUFFER_H