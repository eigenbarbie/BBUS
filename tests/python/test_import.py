import unittest


class ImportTest(unittest.TestCase):
    def test_extension_exposes_a_version(self) -> None:
        import belief_update

        self.assertEqual(belief_update.__version__, "0.1.0")


if __name__ == "__main__":
    unittest.main()
