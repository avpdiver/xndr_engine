#pragma once

[[nodiscard]] void *operator new(size_t size);
[[nodiscard]] void *operator new[](size_t size);

void operator delete(void *p) noexcept;
void operator delete(void *ptr, size_t size) noexcept;
void operator delete[](void *ptr) noexcept;
void operator delete[](void *ptr, size_t size) noexcept;