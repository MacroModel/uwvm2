(module
  (func $leaf (param i32) (result i32)
    local.get 0
    i32.const 7
    i32.xor)

  (func $pair_xor (param i32 i32) (result i32)
    local.get 0
    local.get 1
    i32.xor)

  (func $mid (param i32) (result i32)
    local.get 0
    call $leaf
    i32.const 5
    i32.add)

  (func $mid_append_const (param i32) (result i32)
    local.get 0
    i32.const 10
    call $pair_xor)

  (func (export "_start")
    i32.const 9
    call $mid
    i32.const 12
    call $mid_append_const
    i32.add
    i32.const 19
    i32.const 6
    i32.add
    i32.ne
    if
      unreachable
    end))
