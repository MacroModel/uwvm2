(module
  (import "jit_provider" "bump" (func $bump (param i32) (result i32)))

  (func $call_bump_const (result i32)
    i32.const 1
    call $bump)

  (func (export "_start")
    call $call_bump_const
    i32.const 42
    i32.ne
    if
      unreachable
    end))
