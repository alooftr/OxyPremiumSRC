#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "field.hpp"

class Writer
{
	std::vector<std::vector<Field>> fields;
public:
	Writer() {}
	Writer(size_t maxFields) {
		size_t vector_size = maxFields + 1;
		fields.resize(vector_size);
		fields.reserve(vector_size);
	}

	Writer(void* data, size_t size, size_t maxFields)
		: Writer(maxFields)
	{
		const size_t vector_size = maxFields + 1;
		size_t
			pos = 0,
			bytesRead;

		if (data == nullptr)
			return;
		// parse packet
		while (pos < size) {
			auto field = Field::read((void*)((ptrdiff_t)data + pos), bytesRead);
			if (!bytesRead) break;

			auto index = field.tag.field;
			if (index >= vector_size) throw("fields range error: field[%i]", index);
			fields[index].push_back(field);
			pos += bytesRead;
		}
	}

	Writer(std::string dataStr, size_t maxFields)
		: Writer((void*)dataStr.data(), dataStr.size(), maxFields) {}

	std::string serialize() {
		std::string result;
		for (auto& f0 : fields) {
			for (auto& f1 : f0) {
				result += f1.full;
			}
		}
		return result;
	}

	void print() {
		auto data = serialize();
		void* mem = (void*)data.data();
		size_t size = data.size();
		int j = 0;
		for (int i = 0; i <= size; ++i) {
			printf("%.2X ", *(unsigned char*)((uintptr_t)mem + i));
			j++;
			if (j == 16)
			{
				j = 0;
				printf("\n");
			}
		}
		printf("\n");
	}

	void add(Field field) {
		fields[field.tag.field].push_back(field);
	}

	void replace(Field field) {
		fields[field.tag.field].clear();
		fields[field.tag.field].push_back(field);
	}

	void replace(Field field, uint32_t index) {
		fields[field.tag.field][index] = field;
	}

	void clear(unsigned fieldId) {
		return fields[fieldId].clear();
	}

	bool has(unsigned fieldId) {
		return fields[fieldId].size() > 0;
	}

	Field get(unsigned fieldId)
	{
		return fields[fieldId][0];
	}

	std::vector<Field> getAll(unsigned fieldId)
	{
		return fields[fieldId];
	}
};
