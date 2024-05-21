#pragma once

// A simple status.
//
enum class CondicionSimple : uint8_t {
  Exito,
  Error
};

// Check if a function was executed successfully.
//
// @return `true` if the function was executed successfully.
//
template<typename CondicionEnum>
constexpr bool satisfactorio(CondicionEnum condicion) {
  return condicion == CondicionEnum::Exito;
}

// Check if a function failed with any error.
//
// @return `true` if the function returned anything else than `Exito`.
//
template<typename CondicionEnum>
constexpr bool fallido(CondicionEnum condicion) {
  return condicion != CondicionEnum::Exito;
}