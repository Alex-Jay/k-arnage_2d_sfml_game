#pragma once
#include <map>
#include <string>
#include <memory>
#include <stdexcept>
#include <cassert>

template<typename Resource, typename Identifier>
class ResourceHolder
{
private:
	std::map<Identifier, std::unique_ptr<Resource>> mResourceMap;

private:
	void insertResource(Identifier id, std::unique_ptr<Resource> resource);

public:
	void load(Identifier id, const std::string& filename);
	template<typename Parameter>
	void load(Identifier id, const std::string& filename, const Parameter& secondParameter);
	Resource& get(Identifier);
	const Resource& get(Identifier) const;
};

#include "ResourceHolder.inl"