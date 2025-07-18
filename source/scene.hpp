#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <opencv2/opencv.hpp>

#include "view.hpp"
#include "project.hpp"



class Box;
class Floor;
class Frame;
class Volume;
class Frustum;
class Checkers;


/**
 * @class Scene
 * @brief Manages all 3D models and their relationships in the scene.
 */
class Scene {
public: // Constructors
	/** @brief Construct a new Scene object. */
	Scene()
	: box_(nullptr)
	, floor_(nullptr)
	, frame_(nullptr)
	, volume_(nullptr)
	, checkers_(nullptr) {}

public:
	/**
	 * @brief Load a project and initialize scene models.
	 * @param project Shared pointer to the project to load.
	 */
	void load_project(std::shared_ptr<Project> project);

	/** @brief Unload the current project and reset the scene. */
	void unload_project();

public: // Getters
	/**
	 * @brief Get the box model.
	 * @return Shared pointer to Box.
	 */
	std::shared_ptr<Box> box() const { return box_; }

	/**
	 * @brief Get the floor model.
	 * @return Shared pointer to Floor.
	 */
	std::shared_ptr<Floor> floor() const { return floor_; }

	/**
	 * @brief Get the frame model.
	 * @return Shared pointer to Frame.
	 */
	std::shared_ptr<Frame> frame() const { return frame_; }

	/**
	 * @brief Get the volume model.
	 * @return Shared pointer to Volume.
	 */
	std::shared_ptr<Volume> volume() const { return volume_; }

	/**
	 * @brief Get the checkers model.
	 * @return Shared pointer to Checkers.
	 */
	std::shared_ptr<Checkers> checkers() const { return checkers_; }

	/**
	 * @brief Get the camera frustums.
	 * @return Vector of shared pointers to Frustum.
	 */
	const std::vector<std::shared_ptr<Frustum>>& frustums() const { return frustums_; }

private:
	/** @brief Create the box model. */
	void create_box();

	/** @brief Create the floor model. */
	void create_floor();

	/** @brief Create the frame model. */
	void create_frame();

	/** @brief Create the checkers model. */
	void create_checkers(int rows, int cols, float square_size);

	/** @brief Create camera frustums for the given views. */
	void create_frustums(const std::vector<View>& views);

	/** @brief Create an empty volume for the initial state. */
	void create_empty_volume();

	/** @brief Create the volume model for the given views. */
	void create_volume(const std::vector<View>& views);

private:
	std::shared_ptr<Box> box_;
	std::shared_ptr<Floor> floor_;
	std::shared_ptr<Frame> frame_;
	std::shared_ptr<Volume> volume_;
	std::shared_ptr<Checkers> checkers_;
	std::vector<std::shared_ptr<Frustum>> frustums_;
};
