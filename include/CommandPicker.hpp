#pragma once

#include "CommandManager.hpp"
#include "Config.hpp"
#include "Picker.hpp"

#include <functional>
#include <vector>

class CommandManager;

class CommandPicker : public Picker
{
    Q_OBJECT
public:
    using ShortcutMap = QHash<QString, QStringList>;
    explicit CommandPicker(const Config::CommandPalette &config,
                           const std::vector<Command> &commands,
                           const ShortcutMap &shortcuts,
                           CommandManager *command_manager,
                           QWidget *parent) noexcept;

    void registerCommand(const QString &name, const QStringList &shortcuts,
                         std::function<void()> action);

    // Picker interface
    QList<Item> collectItems() override;
    void onItemAccepted(const Item &item) override;

private:
    const Config::CommandPalette &m_config;
    const std::vector<Command> m_commands; // sorted, stable
    const ShortcutMap &m_shortcuts;
    // Used to record per-command usage (on accept) and to read usage
    // counts back when config.sort_by_frequency is on, so the most-used
    // commands float to the top of the list — smex-style. May be null;
    // frequency sorting/recording is simply skipped then.
    CommandManager *m_command_manager;
};
