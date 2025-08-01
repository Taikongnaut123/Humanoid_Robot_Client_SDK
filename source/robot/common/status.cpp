/**
 * Copyright (c) 2025 Humanoid Robot, lnc.  All rights reserved.
 */

#include "status.h"
#include "success_condition.h"
#include <iostream>
using namespace humanoid_robot::common;

Status::Status(const std::error_code &code, std::string message)
    : m_code(code), m_message(std::move(message)) {}

Status Status::Chain(std::string message) const
{
    message += ": ";
    message += m_message;
    return Status(m_code, std::move(message));
}

Status Status::Chain(std::error_code code, std::string message) const
{
    message += ": ";
    message += DebugString();
    return Status(code, std::move(message));
}

std::string Status::DebugString() const
{
    std::string result = std::to_string(m_code.value());
    result += "(";
    result += m_code.message();
    result += "): ";
    result += m_message;
    return result;
}

Status::operator bool() const
{
    return !m_code; // 按照标准约定，0 值表示成功，非 0 值表示错误
}
