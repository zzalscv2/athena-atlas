#!/bin/bash
less $1 | grep "<Run>" |
cut -c 12-17