# [Documentation](README.md) > Design

## Server

TBC

## Worker

The worker is a single-threaded process designed to handle some of the
servers workload. Communication between the worker and the server is only in one
direction - i.e. the worker is only _reading_ data from the server.

TBC

## Journal

A journal is only managed by _one_ worker. This worker is 

## Transaction