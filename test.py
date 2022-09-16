#!/usr/bin/env python3

import unittest
import subprocess

class Test(unittest.TestCase):
    def setUp(self):
        self.p = subprocess.Popen([
            'qemu-system-aarch64',
            '-M',
            'stm32vldiscovery',
            '-m',
            '128M',
            '-nographic',
            '-no-reboot',
            '-d',
            'in_asm,int,exec,cpu,guest_errors,unimp,cpu_reset',
            '-D',
            'qemu.log',
            '-kernel',
            'out.bin'
        ], stdin=subprocess.PIPE, stdout=subprocess.PIPE)

    def tearDown(self):
        if self.p.returncode is None:
            self.p.kill()
            self.p.wait()

        self.p.stdin.close()
        self.p.stdout.close()

    def _write_to_proc(self, s):
        c = self.p.stdout.read(1)
        self.assertEqual(c, b">")

        for e in bytearray(s, "utf-8"):
            self.p.stdin.write(bytes([ e ]))
            self.p.stdin.flush()
            c = self.p.stdout.read(1)
            self.assertEqual(c, bytes([ e ]))

    def test_reset(self):
        self._write_to_proc("x\n")
        self.p.wait(1)

        self.assertEqual(self.p.returncode, 0)

    def test_task(self):
        self._write_to_proc("$\n")

        buffer = b""
        c = self.p.stdout.read(1)
        while c != b'\n':
            buffer += c
            c = self.p.stdout.read(1)

        self.assertEqual(buffer.decode(), "0")

    def test_test_1(self):
        self._write_to_proc("t1 4 1000\n")

        buffer = b""
        c = self.p.stdout.read(1)
        while c != b'\n':
            buffer += c
            c = self.p.stdout.read(1)

        self.assertEqual(buffer.decode(), "1003")
