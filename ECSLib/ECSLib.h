#pragma once
#include <type_traits>
#include <string>
#include <memory>
#include <bitset>
#include <unordered_map>
#include <queue>
#include <iostream>
#include <set>
#include <boost/dynamic_bitset.hpp>

// Dummy class to be derived by singleton components
class Singleton {

};

// Component Array Class:
// Stores an array of components and provides functions for accessing them.
class ICompArray {
public:
	virtual void CreateComponent(int o) = 0;
	virtual void DestroyComponent(int o) = 0;

protected:
	std::unordered_map<int, int> objectIDs;
	std::queue<int> availableIDs;
};
template <class T> class CompArray : public ICompArray {
public:
	void CreateComponent(int o) {
		if (availableIDs.empty()) {
			components.push_back(T());
			objectIDs.insert({ o,components.size() - 1 });
		}
		else {
			components[availableIDs.front()] = T();
			objectIDs.insert({ o,availableIDs.front() });
			availableIDs.pop();
		}
	}

	void DestroyComponent(int o) {
		GetComponent(o) = T();
		availableIDs.push(objectIDs[o]);
		objectIDs.erase(o);
	}

	T& GetComponent(int o) {
		return components[objectIDs[o]];
	}

	int size() { return availableIDs.size(); }

private:
	std::vector<T> components;
};

// CompArrays Class:
// A container for the component arrays. Provides functions for accessing a component
// array of a given type as well as its contents.
class CompArrays {
public:
	template<class...Ts> typename std::enable_if<sizeof...(Ts) == 0>::type RegisterComponent() {}
	template<class T, class...Ts> void RegisterComponent() {
		compIDs[typeid(T).name()] = nextCompID;
		nextCompID++;
		compArrays[compIDs[typeid(T).name()]] = std::make_shared<CompArray<T>>();
		RegisterComponent<Ts...>();
	}

	template <class T> T& GetComponent(int o) {
		return GetComponentArray<T>()->GetComponent(o);
	}

	template <class T> std::shared_ptr<CompArray<T>> GetComponentArray() {
		return std::static_pointer_cast<CompArray<T>>(compArrays[compIDs[typeid(T).name()]]);
	}

	std::shared_ptr<ICompArray> GetComponentArray(int c) {
		return std::static_pointer_cast<ICompArray>(compArrays[c]);
	}

	template <class T> int GetCompID() {
		return compIDs[typeid(T).name()];
	}

	int NumberComponents() {
		return nextCompID;
	}

private:
	std::unordered_map<const char*, int> compIDs;
	std::unordered_map<int, std::shared_ptr<ICompArray>> compArrays;
	int nextCompID = 0;
};

// Object Class:
// Represents an object in the game, this class is used to access its components.
class Object {
public:
	bool operator==(const Object& other) const {
		return this->id == other.id;
	}

	bool operator<(const Object& other) const {
		return this->id < other.id;
	}

	template <class T> T& GetComponent() {
		return compArrays->GetComponent<T>(id);
	}

	template <class T> T& GetComponent() const {
		return const_cast<T&>(compArrays->GetComponent<T>(id));
	}

private:
	int id;
	CompArrays* compArrays;

	friend class GameData;
};

// EventInterface Class:
// Not to be confused with the user-derived GInterface, this class tracks
// whether scenes should be switched or the game should be exited.
class EventInterface {
public:
	void QuitGame() {
		quit = true;
	}
	bool ShouldQuit() {
		return quit;
	}
	void SwitchScene(std::string _scene) {
		scene = _scene;
		switchScene = true;
	}
	std::pair<bool, std::string> ShouldSwitchScene() {
		return { switchScene,scene };
	}
	void Reset() {
		quit = false;
		switchScene = false;
	}

private:
	bool quit = false;
	bool switchScene = false;
	std::string scene;
};

// GameData Class:
// This class stores the data for a game including its component arrys, objects and groups 
// of objects, singletons, persistent singletons, and object definitions. This is the core
// of the game.
class GameData {
public:
	template<class...Ts> typename std::enable_if<sizeof...(Ts) == 0>::type CreateSingletons() {}
	template<class T, class...Ts> void CreateSingletons() {
		singletons[typeid(T).name()] = std::make_shared<T>();
		CreateSingletons<Ts...>();
	}

	template <class T> T& GetSingleton() {
		return *(std::static_pointer_cast<T>(singletons[typeid(T).name()]));
	}
	template <class T> T& GetPersistentSingleton() {
		return *(std::static_pointer_cast<T>(persistentSingletons->at(typeid(T).name())));
	}

	template<class...Ts> void RegisterComponent() { compArrays.RegisterComponent<Ts...>(); }

	template<class...Ts> typename std::enable_if<sizeof...(Ts) == 0>::type DefineObject(std::string name) {}
	template<class T, class...Ts> void DefineObject(std::string name) {
		objectDefinitions[name].resize(compArrays.NumberComponents());
		objectDefinitions[name].set(GetCompID<T>());
		DefineObject<Ts...>(name);
	}

	Object CreateObject(std::string name) {
		int o;
		boost::dynamic_bitset<> signature = objectDefinitions[name];
		if (availableObjIDs.empty()) {
			objectSignatures.push_back(signature);
			o = objectSignatures.size() - 1;
		}
		else {
			o = availableObjIDs.front();
			availableObjIDs.pop();
			objectSignatures[o] = signature;
		}

		for (int i = 0; i < signature.size(); i++) {
			if (signature[i]) {
				GetComponentArray(i)->CreateComponent(o);
			}
		}
		for (int i = 0; i < groupSigs.size(); i++) {
			if ((groupSigs[i] & signature) == groupSigs[i]) {
				groups[i].insert(ConstructObject(o));
			}
		}
		return ConstructObject(o);
	}

	void DestroyObject(Object e) {
		int o = e.id;
		boost::dynamic_bitset<> sig = objectSignatures[o];
		for (int i = 0; i < sig.size(); i++) {
			if (sig[i]) {
				GetComponentArray(i)->DestroyComponent(o);
			}
		}
		for (int i = 0; i < groupSigs.size(); i++) {
			if ((groupSigs[i] & sig) == groupSigs[i]) {
				Object temp = Object();
				temp.id = o;
				groups[i].erase(temp);
			}
		}
		for (auto& tagSet : tags) {
			tagSet.second.erase(ConstructObject(o));
		}
		availableObjIDs.push(o);
		objectSignatures[o].reset();
	}

	void SetPersistentSingletons(GameData* data) {
		persistentSingletons = data->persistentSingletons;
	}

	template <class T> T& GetComponent(int e) {
		return GetComponentArray<T>()->GetComponent(e);
	}

	template<class ...Ts> std::set<Object>& ObjectsWith() {
		boost::dynamic_bitset<> sig = GetSignature<Ts...>();
		if (!groupInit[sig]) {
			groupSigs.push_back(sig);
			groups.push_back(std::set<Object>());
			groupIDs[sig] = nextGroupID;
			for (int i = 0; i < objectSignatures.size(); i++) {
				if (ObjInGroup(i, nextGroupID)) {
					groups[nextGroupID].insert(ConstructObject(i));
				}
			}
			groupInit[sig] = true;
			nextGroupID++;
			return (groups[groupIDs[sig]]);
		}
		else {
			return (groups[groupIDs[sig]]);
		}
	}

	std::set<Object>& ObjectsWith(std::string tag) {
		return tags[tag];
	}

	bool ObjInGroup(int e, int g) {
		return (groupSigs[g] & objectSignatures[e]) == groupSigs[g];
	}

	template <class ...Ts> int GetGroupID() {
		return groupIDs[GetSignature<Ts...>()];
	}

	void AddTag(Object o, std::string tag) {
		tags[tag].insert(o);
	}

	EventInterface eventInterface;

private:
	template <class T> std::shared_ptr<CompArray<T>> GetComponentArray() {
		return compArrays.GetComponentArray<T>();
	}

	std::shared_ptr<ICompArray> GetComponentArray(int c) {
		return compArrays.GetComponentArray(c);
	}

	template <class T> int GetCompID() {
		return compArrays.GetCompID<T>();
	}

	Object ConstructObject(int id) {
		Object o = Object();
		o.id = id;
		o.compArrays = &compArrays;
		return o;
	}

	template<class...Ts> boost::dynamic_bitset<> GetSignature() {
		return GetSignature<Ts...>(boost::dynamic_bitset<>(compArrays.NumberComponents()));
	}
	template<class...Ts> typename std::enable_if<sizeof...(Ts) == 0, boost::dynamic_bitset<>>::type GetSignature(boost::dynamic_bitset<> sig) {
		return sig;
	}
	template<class T, class...Ts> boost::dynamic_bitset<> GetSignature(boost::dynamic_bitset<> sig) {
		sig.set(GetCompID<T>());
		return GetSignature<Ts...>(sig);
	}

	//Singletons
	std::unordered_map<const char*, std::shared_ptr<Singleton>> singletons;
	std::shared_ptr<std::unordered_map<const char*, std::shared_ptr<Singleton>>> persistentSingletons;
	//Components
	CompArrays compArrays;
	//Objects
	std::queue<int> availableObjIDs;
	std::vector<boost::dynamic_bitset<>> objectSignatures;
	std::unordered_map<std::string, boost::dynamic_bitset<>> objectDefinitions;
	//Groups
	std::vector<boost::dynamic_bitset<>> groupSigs;
	std::unordered_map<boost::dynamic_bitset<>, int> groupIDs;
	int nextGroupID = 0;
	std::unordered_map<boost::dynamic_bitset<>, bool> groupInit;
	std::vector<std::set<Object>> groups;
	//Tags
	std::unordered_map<std::string, std::set<Object>> tags;

	friend class Game;
};

// Forward-declare the GInterface class so it can reference pointers to itself
class GInterface;

// InterfaceStorer Class:
// Class for storing and accessing the interfaces used by the game.
class InterfaceStorer {
public:
	template<class...Ts> typename std::enable_if<sizeof...(Ts) == 0>::type CreateInterfaces(GameData* gdata) {}
	template<class T, class...Ts> void CreateInterfaces(GameData* gdata) {
		interfaces[typeid(T).name()] = std::make_shared<T>();
		interfaces[typeid(T).name()]->gdata = gdata;
		CreateInterfaces<Ts...>(gdata);
	}

	template <class T> T& GetInterface() {
		//return static_cast<T>(*(interfaces[typeid(T).name()]));
		return *(std::static_pointer_cast<T>(interfaces[typeid(T).name()]));
	}

private:
	std::unordered_map<const char*, std::shared_ptr<GInterface>> interfaces;
};

// Interface Class:
// Derived by the user to access and modify the game state.
class GInterface {
protected:
	Object CreateObject(std::string name) {
		return gdata->CreateObject(name);
	}

	void AddTag(Object o, std::string tag) {
		gdata->AddTag(o, tag);
	}

	void DestroyObject(Object o) {
		deleteQueue.insert(o);
	}

	template <class T> T& GetInterface() {
		return interfaces->GetInterface<T>();
	}

	template<class ...Ts> std::set<Object>& ObjectsWith() {
		return gdata->ObjectsWith<Ts...>();
	}

	template <class T> T& GetSingleton() {
		return gdata->GetSingleton<T>();
	}
	template <class T> T& GetPersistentSingleton() {
		return gdata->GetPersistentSingleton<T>();
	}

	void QuitGame() {
		gdata->eventInterface.QuitGame();
	}
	void SwitchScene(std::string scene) {
		gdata->eventInterface.SwitchScene(scene);
	}

private:
	GameData* gdata;
	InterfaceStorer* interfaces;
	std::set<Object> deleteQueue;
	friend class InterfaceStorer;
	friend class System;
	friend class Scene;
};

// System Class:
// A class with single update function that can be called to update the game state in a 
// particular way. To be derived by the user and registered in a scene.
class System : public GInterface {
public:
	virtual void Update() {};
	virtual void Update(float dt) {
		Update();
	};

protected:
	template <class T> T& GetInterface() {
		return interfaces->GetInterface<T>();
	}

	template <class T> T& GetSingleton() {
		return gdata->GetSingleton<T>();
	}

	using GInterface::ObjectsWith;

	std::set<Object> ObjectsWith(std::string tag) {
		return gdata->ObjectsWith(tag);
	}

private:
	InterfaceStorer* interfaces;
	friend class Scene;
};

// Scene Class:
// The Scene is where the user-derived types are to be registered and the initial objects are created.
// A game can consist of multiple scenes that are switched between.
class Scene {
public:
	virtual void Start() {
		Init();
		bool quit = false;
		while (!quit) {
			for (int i = 0; i < defaultSystems.size(); i++) {
				defaultSystems[i]->Update();
				while (!defaultSystems[i]->deleteQueue.empty()) {
					gameData.DestroyObject(*(defaultSystems[i]->deleteQueue.begin()));
					defaultSystems[i]->deleteQueue.erase(defaultSystems[i]->deleteQueue.begin());
				}
			}
			if (gameData.eventInterface.ShouldQuit() || gameData.eventInterface.ShouldSwitchScene().first) {
				quit = true;
			}
		}
		Quit();
	}

	bool RunBatch(std::string batch) {
		bool quit = false;
		for (int i = 0; i < systems[batch].size(); i++) {
			systems[batch][i]->Update();
		}
		if (gameData.eventInterface.ShouldQuit() || gameData.eventInterface.ShouldSwitchScene().first) {
			quit = true;
		}
		return quit;
	}

	bool RunBatch(std::string batch, float dt) {
		bool quit = false;
		for (int i = 0; i < systems[batch].size(); i++) {
			systems[batch][i]->Update(dt);
		}
		if (gameData.eventInterface.ShouldQuit() || gameData.eventInterface.ShouldSwitchScene().first) {
			quit = true;
		}
		return quit;
	}

	void SetPersistentGamedata(Scene* other) {
		gameData.SetPersistentSingletons(&other->gameData);
	}

protected:
	virtual void Init() = 0;
	virtual void Quit() { };

	template <class T> T& GetSingleton() {
		return gameData.GetSingleton<T>();
	}
	template <class T> T& GetPersistentSingleton() {
		return gameData.GetPersistentSingleton<T>();
	}

	template<class...Ts> void RegisterComponents() {
		gameData.RegisterComponent<Ts...>();
	}

	template <class T> void CreateSingleton() {
		gameData.CreateSingletons<T>();
	}

	template <class...Ts> void CreateInterfaces() {
		interfaces.CreateInterfaces<Ts...>(&gameData);
		LoadInterfaces<Ts...>();
	}
	template <class...Ts> typename std::enable_if<sizeof...(Ts) == 0>::type LoadInterfaces() {};
	template <class T, class...Ts> void LoadInterfaces() {
		static_cast<GInterface&>(interfaces.GetInterface<T>()).interfaces = &interfaces;
		LoadInterfaces<Ts...>();
	}

	template <class T> T& GetInterface() {
		return interfaces.GetInterface<T>();
	}

	template<class...Ts> typename std::enable_if<sizeof...(Ts) == 0>::type RegisterSystems(std::string batch) {}
	template<class T, class...Ts> void RegisterSystems(std::string batch = "") {
		auto sys = std::make_shared<T>();
		sys->gdata = &gameData;
		sys->interfaces = &interfaces;
		if (batch == "") {
			defaultSystems.push_back(sys);
		}
		else {
			systems[batch].push_back(sys);
		}
		RegisterSystems<Ts...>(batch);
	}

	template<class...Ts> void DefineObject(std::string name) {
		gameData.DefineObject<Ts...>(name);
	}

	Object CreateObject(std::string name) {
		return gameData.CreateObject(name);
	}

	void AddTag(Object o, std::string tag) {
		gameData.AddTag(o, tag);
	}

	void Reset() {
		systems = std::unordered_map<std::string, std::vector<std::shared_ptr<System>>>();
		defaultSystems = std::vector<std::shared_ptr<System>>();
		interfaces = InterfaceStorer();
		gameData = GameData();
	}

private:
	std::unordered_map<std::string, std::vector<std::shared_ptr<System>>> systems;
	std::vector<std::shared_ptr<System>> defaultSystems;
	InterfaceStorer interfaces;
	GameData gameData;

	friend class Game;
};

// Game Class:
// The highest-level class, used to register scenes so that they can switch between each other. This is where 
// the game is started from.
class Game {
public:
	Game() {
		persistentSingletons = std::make_shared<std::unordered_map<const char*, std::shared_ptr<Singleton>>>();
	}

	void Start(std::string _scene) {
		scene = _scene;
		Init();
		scenes[scene]->Start();
		while (!scenes[scene]->gameData.eventInterface.ShouldQuit() && scenes[scene]->gameData.eventInterface.ShouldSwitchScene().first) {
			scenes[scene]->gameData.eventInterface.Reset();
			std::string temp = scene;
			scene = scenes[scene]->gameData.eventInterface.ShouldSwitchScene().second;
			scenes[temp]->Reset();
			scenes[temp]->gameData.persistentSingletons = persistentSingletons;
			scenes[scene]->Start();
		}
		Quit();
	}
protected:
	virtual void Init() = 0;
	virtual void Quit() { };
	template<class T>
	void RegisterScene(std::string name) {
		scenes[name] = std::make_shared<T>();
		scenes[name]->gameData.persistentSingletons = persistentSingletons;
	}
	template<class T>
	T& GetScene(std::string name) {
		return *std::static_pointer_cast<T>(scenes[name]);
	}

	template<class...Ts> typename std::enable_if<sizeof...(Ts) == 0>::type CreatePersistentSingletons() {}
	template<class T, class...Ts> void CreatePersistentSingletons() {
		persistentSingletons->insert({ typeid(T).name(),std::make_shared<T>() });
		CreatePersistentSingletons<Ts...>();
	}
	template <class T> T& GetPersistentSingleton() {
		return *(std::static_pointer_cast<T>(persistentSingletons->at(typeid(T).name())));
	}

private:
	std::shared_ptr<std::unordered_map<const char*, std::shared_ptr<Singleton>>> persistentSingletons;
	std::unordered_map<std::string, std::shared_ptr<Scene>> scenes;
	std::string scene;
};