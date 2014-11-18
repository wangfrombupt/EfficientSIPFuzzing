-- Database for P-CSCF and S-CSCF persistency

CREATE DATABASE IF NOT EXISTS pscscf;

USE pscscf;

DROP TABLE IF EXISTS snapshot;
CREATE TABLE snapshot (
	id INT NOT NULL AUTO_INCREMENT,
	node_id VARCHAR(64) NOT NULL,
	data_type INT NOT NULL,
    snapshot_version INT NOT NULL,
    step_version INT NOT NULL,
    record_id_1 VARCHAR(256),
    record_id_2 VARCHAR(256),
    record_id_3 VARCHAR(256),
    record_id_4 VARCHAR(256),
    data BLOB NOT NULL,
    PRIMARY KEY(id)
);

grant delete,insert,select,update on pscscf.* to pscscf@localhost identified by 'pscscf';