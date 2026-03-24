(module
  (type $dispatch_t (func (param i32) (result i32)))
  (table 2 funcref)

  (func $add11 (param i32) (result i32)
    local.get 0
    i32.const 11
    i32.add)

  (func $mul3 (param i32) (result i32)
    local.get 0
    i32.const 3
    i32.mul)

  (elem (i32.const 0) $add11 $mul3)

  (func $dispatch (param i32 i32) (result i32)
    local.get 0
    local.get 1
    call_indirect (type $dispatch_t))

  (func $dispatch_const0 (param i32) (result i32)
    local.get 0
    i32.const 0
    call_indirect (type $dispatch_t))

  (func $dispatch_const1_mul2 (param i32) (result i32)
    local.get 0
    i32.const 1
    call_indirect (type $dispatch_t)
    i32.const 2
    i32.mul)

  (func (export "_start")
    i32.const 7
    i32.const 0
    call $dispatch
    i32.const 5
    i32.const 1
    call $dispatch
    i32.add
    i32.const 9
    call $dispatch_const0
    i32.add
    i32.const 6
    call $dispatch_const1_mul2
    i32.add
    i32.const 89
    i32.ne
    if
      unreachable
    end))
