#!/bin/bash
find /var/db/pkg -maxdepth 1 -not -path '*/[@.]*' -type d -printf "%f\n"
