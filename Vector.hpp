#pragma once

#include <algorithm>
#include <vector>

/*
 * Wrapping std::vector, because I kinda like the QVector's interface,
 * but the thing doesn't support move-semantics in reallocs.
 * Might revert to QVector some time in the future, when Qt finally advances
 * to 21st century.
 */
template <typename T>
class Vector {
public:
	Vector() = default;
	Vector(Vector &&other) = default;
	Vector(std::initializer_list <T> elems) : m_data{elems} {}
	~Vector() = default;

	Vector & operator = (Vector &&) = default;

	void clear() noexcept { m_data.clear(); }
	bool contains(const T &value) const noexcept { return std::find(m_data.begin(), m_data.end(), value) != m_data.end(); }
	int count() const noexcept { return m_data.size(); }
	bool empty() const noexcept { return m_data.empty(); }

	T & front() noexcept { return m_data.front(); }
	const T & front() const noexcept { return m_data.front(); }

	T & back() noexcept { return m_data.back(); }
	const T & back() const noexcept { return m_data.back(); }

	template <typename TT>
	void push_back(TT &&value) noexcept { m_data.push_back(std::forward<TT>(value)); }
	void pop_back() noexcept { m_data.pop_back(); }

	decltype(auto) begin() noexcept { return m_data.begin(); }
	decltype(auto) begin() const noexcept { return m_data.begin(); }
	decltype(auto) cbegin() const noexcept { return m_data.begin(); }

	decltype(auto) end() noexcept { return m_data.end(); }
	decltype(auto) end() const noexcept { return m_data.end(); }
	decltype(auto) cend() const noexcept { return m_data.end(); }

private:
	std::vector <T> m_data;
};
