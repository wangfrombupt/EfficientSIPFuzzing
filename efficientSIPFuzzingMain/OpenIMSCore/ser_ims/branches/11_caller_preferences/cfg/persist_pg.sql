CREATE TABLE snapshot (
	id SERIAL PRIMARY KEY,
	node_id VARCHAR(64) NOT NULL,
	data_type INTEGER NOT NULL,
    snapshot_version INTEGER NOT NULL,
    step_version INTEGER NOT NULL,
    record_id_1 VARCHAR(256),
    record_id_2 VARCHAR(256),
    record_id_3 VARCHAR(256),
    record_id_4 VARCHAR(256),
    data BYTEA NOT NULL
);