/*
 * Rosalie's Mupen GUI - https://github.com/Rosalie241/RMG
 *  Copyright (C) 2020-2025 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#define CORE_INTERNAL
#include "rmgk_ggpo.hpp"

#include "Library.hpp"

namespace
{
rmgk_ggpo::SessionCallbacks g_SessionCallbacks;
void* g_SynchronizeInputUserData = nullptr;
bool g_SessionRunning = false;

int rmgk_ggpo_core_input_callback(void* values, int size, int players)
{
    return rmgk_ggpo::synchronize_input(values, size, players) ? 1 : 0;
}
} // namespace

CORE_EXPORT bool rmgk_ggpo::start_session(const SessionCallbacks& callbacks, void* userData)
{
    g_SessionCallbacks = callbacks;
    g_SynchronizeInputUserData = userData;
    g_SessionRunning = true;

    if (!install_core_input_callback())
    {
        g_SessionCallbacks = {};
        g_SynchronizeInputUserData = nullptr;
        g_SessionRunning = false;
        return false;
    }

    return true;
}

CORE_EXPORT void rmgk_ggpo::close_session()
{
    clear_core_input_callback();
}

CORE_EXPORT bool rmgk_ggpo::idle(int timeoutMs)
{
    (void)timeoutMs;
    return g_SessionRunning;
}

CORE_EXPORT bool rmgk_ggpo::is_session_running()
{
    return g_SessionRunning;
}

CORE_EXPORT bool rmgk_ggpo::save_game_state(CoreRollbackState& state, int frame)
{
    return CoreRollbackSaveGameState(state, frame);
}

CORE_EXPORT bool rmgk_ggpo::load_game_state(const CoreRollbackState& state)
{
    return CoreRollbackLoadGameState(state);
}

CORE_EXPORT void rmgk_ggpo::free_buffer(CoreRollbackState& state)
{
    CoreRollbackFreeGameState(state);
}

CORE_EXPORT bool rmgk_ggpo::advance_frame(int frameOutputFlags)
{
    return CoreRunFrames(1, frameOutputFlags);
}

CORE_EXPORT bool rmgk_ggpo::advance_frames(int frames, int frameOutputFlags)
{
    return CoreRunFrames(frames, frameOutputFlags);
}

CORE_EXPORT bool rmgk_ggpo::set_deterministic(bool enabled)
{
    return CoreRollbackSetDeterministic(enabled);
}

CORE_EXPORT void rmgk_ggpo::set_synchronize_input_callback(SynchronizeInputCallback callback, void* userData)
{
    g_SessionCallbacks.synchronize_input = callback;
    g_SynchronizeInputUserData = userData;
}

CORE_EXPORT bool rmgk_ggpo::install_core_input_callback()
{
    return CoreRollbackSetInputCallback(rmgk_ggpo_core_input_callback);
}

CORE_EXPORT void rmgk_ggpo::clear_core_input_callback()
{
    CoreRollbackSetInputCallback(nullptr);
    g_SessionCallbacks = {};
    g_SynchronizeInputUserData = nullptr;
    g_SessionRunning = false;
}

CORE_EXPORT bool rmgk_ggpo::synchronize_input(void* values, int size, int players)
{
    if (g_SessionCallbacks.synchronize_input == nullptr)
    {
        return true;
    }

    return g_SessionCallbacks.synchronize_input(values, size, players, g_SynchronizeInputUserData);
}
