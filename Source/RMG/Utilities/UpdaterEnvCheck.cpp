/*
 * Rosalie's Mupen GUI - https://github.com/Rosalie241/RMG
 *  Copyright (C) 2020-2025 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "UpdaterEnvCheck.hpp"
#include "EnvVars.hpp"

#include <QtEnvironmentVariables>

using namespace Utilities;

bool UpdaterEnvCheck::ShouldAllowUpdater()
{
    bool allowed = qEnvironmentVariableIntegerValue(EnvVars::RMGDisableUpdater) != 1;
}
