#pragma once

#include <iostream>
#include <filesystem>

#include "yaml-cpp/opencv_yaml.h"
#include "yaml-cpp/qt_yaml.h"
#include "yaml-cpp/qt_custom_yaml.h"
#include "yaml-cpp/torch_yaml.h"

/**
 * Singleton App Config manager.
 */
class AppConfig {
private:
    static inline YAML::Node config_;
    static inline std::filesystem::path config_filepath_;

public:
    static inline YAML::Node &load(const std::string &filename) {
        AppConfig::config_ = YAML::LoadFile(filename);
        if (!AppConfig::config_.IsNull())
            AppConfig::config_filepath_ = std::filesystem::canonical(filename);
        return instance();
    }

    static inline YAML::Node &instance() {
        if (AppConfig::config_.IsNull())
            throw std::runtime_error("config file has not been loaded yet");
        return AppConfig::config_;
    }

    static inline std::filesystem::path config_filepath() {
        return AppConfig::config_filepath_;
    }

    // inner classes to convert relative paths
    struct relative_path : public std::filesystem::path {
        explicit relative_path() : std::filesystem::path() {}

        explicit relative_path(const std::filesystem::path &p) : std::filesystem::path(p) {}
    };

    struct absolute_rel_path : public relative_path {
        explicit absolute_rel_path() : relative_path() {}

        explicit absolute_rel_path(const std::filesystem::path &p) : relative_path(p) {}
    };

    struct canonical_rel_path : public relative_path {
        explicit canonical_rel_path() : relative_path() {}

        explicit canonical_rel_path(const std::filesystem::path &p) : relative_path(p) {}
    };

    using rel_path [[maybe_unused]] = relative_path;
    using arel_path [[maybe_unused]] = absolute_rel_path;
    using crel_path [[maybe_unused]] = canonical_rel_path;
};

/**
 * These converters are used to convert relative path with regards to
 * config_filepath to relative path with regards to app executable.
 */
namespace YAML {
// AppConfig::relative_path
    template<>
    struct convert<AppConfig::relative_path> {
        static Node encode(const AppConfig::relative_path &rhs) {
            return Node(rhs.string());
        }

        static bool decode(const Node &node, AppConfig::relative_path &rhs) {
            if (node.IsNull()) {
            } else if (node.IsScalar()) {
                if (!node.Scalar().empty())
                    rhs = AppConfig::relative_path(
                            std::filesystem::relative(AppConfig::config_filepath().parent_path() / node.Scalar())
                    );
            } else
                return false;
            return true;
        }
    };

    template<>
    struct as_if<AppConfig::relative_path, void> {
        explicit as_if(const Node &node_) : node(node_) {}

        const Node &node;

        AppConfig::relative_path operator()() const {
            if (!node.m_pNode)
                throw TypedBadConversion<AppConfig::relative_path>(node.Mark());

            AppConfig::relative_path t;
            if (convert<AppConfig::relative_path>::decode(node, t))
                return t;
            throw TypedBadConversion<AppConfig::relative_path>(node.Mark());
        }
    };

// AppConfig::absolute_rel_path
    template<>
    struct convert<AppConfig::absolute_rel_path> {
        static Node encode(const AppConfig::absolute_rel_path &rhs) {
            return Node(rhs.string());
        }

        static bool decode(const Node &node, AppConfig::absolute_rel_path &rhs) {
            if (node.IsNull()) {
            } else if (node.IsScalar()) {
                if (!node.Scalar().empty())
                    rhs = AppConfig::absolute_rel_path(
                            std::filesystem::absolute(AppConfig::config_filepath().parent_path() / node.Scalar())
                    );
            } else
                return false;
            return true;
        }
    };

    template<>
    struct as_if<AppConfig::absolute_rel_path, void> {
        explicit as_if(const Node &node_) : node(node_) {}

        const Node &node;

        AppConfig::absolute_rel_path operator()() const {
            if (!node.m_pNode)
                throw TypedBadConversion<AppConfig::absolute_rel_path>(node.Mark());

            AppConfig::absolute_rel_path t;
            if (convert<AppConfig::absolute_rel_path>::decode(node, t))
                return t;
            throw TypedBadConversion<AppConfig::absolute_rel_path>(node.Mark());
        }
    };

// AppConfig::canonical_rel_path
    template<>
    struct convert<AppConfig::canonical_rel_path> {
        static Node encode(const AppConfig::canonical_rel_path &rhs) {
            return Node(rhs.string());
        }

        static bool decode(const Node &node, AppConfig::canonical_rel_path &rhs) {
            if (node.IsNull()) {
            } else if (node.IsScalar()) {
                if (!node.Scalar().empty())
                    rhs = AppConfig::canonical_rel_path(
                            std::filesystem::canonical(AppConfig::config_filepath().parent_path() / node.Scalar())
                    );
            } else
                return false;
            return true;
        }
    };

    template<>
    struct as_if<AppConfig::canonical_rel_path, void> {
        explicit as_if(const Node &node_) : node(node_) {}

        const Node &node;

        AppConfig::canonical_rel_path operator()() const {
            if (!node.m_pNode)
                throw TypedBadConversion<AppConfig::canonical_rel_path>(node.Mark());

            AppConfig::canonical_rel_path t;
            if (convert<AppConfig::canonical_rel_path>::decode(node, t))
                return t;
            throw TypedBadConversion<AppConfig::canonical_rel_path>(node.Mark());
        }
    };
}
