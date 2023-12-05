#!/usr/bin/env python3

import unittest
from icecube import trigger_sim


class DefaultDomsets(unittest.TestCase):
    def testDefaulDomsets(self):
        sets = trigger_sim.GetDefaultDOMSets()

        max_len_set = list()
        for _, v in sets:
            if len(v) > len(max_len_set):
                max_len_set = v

        self.assertEqual(len(max_len_set), 5, "There should be 5 DOM sets in this list not %d." \
                         % len(max_len_set))

        for domset in [2, 4, 5, 6, 11]:
            self.assertTrue(domset in max_len_set, "DOM set %d not found in default lists." % domset)


if __name__ == '__main__':
    unittest.main()
