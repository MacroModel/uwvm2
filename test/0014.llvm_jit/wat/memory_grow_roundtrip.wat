(module
  (memory 1 3)

  (func $grow_and_roundtrip (result i32)
    i32.const 1
    memory.grow
    drop
    i32.const 65536
    i32.const 77
    i32.store
    i32.const 65536
    i32.load)

  (func $roundtrip_plus1 (result i32)
    i32.const 8
    i32.const 9
    i32.store
    i32.const 8
    i32.load
    i32.const 1
    i32.add)

  (func (export "_start")
    call $grow_and_roundtrip
    call $roundtrip_plus1
    i32.add
    i32.const 87
    i32.ne
    if
      unreachable
    end))
