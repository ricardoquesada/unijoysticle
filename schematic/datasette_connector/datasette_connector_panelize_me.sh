#!/bin/bash

python3 ~/src/eagle-brd-merge/merge.py datasette_connector_panelized.brd \
			datasette_connector.brd	               --offy 31.96mm \
			datasette_connector.brd --offx 25.31mm --offy 31.96mm \
			datasette_connector.brd --offx 24.31mm --offy 30.96mm --rotation 180 \
			datasette_connector.brd --offx 49.62mm --offy 30.96mm --rotation 180
