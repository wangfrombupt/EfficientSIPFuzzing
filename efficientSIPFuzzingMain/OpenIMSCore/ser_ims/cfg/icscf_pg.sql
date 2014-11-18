--
-- PostgreSQL database dump
--

SET client_encoding = 'UTF8';
SET check_function_bodies = false;
SET client_min_messages = warning;

--
-- Name: SCHEMA public; Type: COMMENT; Schema: -; Owner: postgres
--

COMMENT ON SCHEMA public IS 'Standard public schema';


SET search_path = public, pg_catalog;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: nds_trusted_domains; Type: TABLE; Schema: public; Owner: icscf; Tablespace: 
--

CREATE TABLE nds_trusted_domains (
    id serial NOT NULL,
    trusted_domain character varying(83) DEFAULT ''::character varying NOT NULL
);


ALTER TABLE public.nds_trusted_domains OWNER TO icscf;

--
-- Name: nds_trusted_domains_id_seq; Type: SEQUENCE SET; Schema: public; Owner: icscf
--

SELECT pg_catalog.setval(pg_catalog.pg_get_serial_sequence('nds_trusted_domains', 'id'), 1, false);


--
-- Name: s_cscf; Type: TABLE; Schema: public; Owner: icscf; Tablespace: 
--

CREATE TABLE s_cscf (
    id serial NOT NULL,
    name character varying(83) DEFAULT ''::character varying NOT NULL,
    s_cscf_uri character varying(83) DEFAULT ''::character varying NOT NULL
);


ALTER TABLE public.s_cscf OWNER TO icscf;

--
-- Name: s_cscf_capabilities; Type: TABLE; Schema: public; Owner: icscf; Tablespace: 
--

CREATE TABLE s_cscf_capabilities (
    id serial NOT NULL,
    id_s_cscf integer DEFAULT 0 NOT NULL,
    capability integer DEFAULT 0 NOT NULL
);


ALTER TABLE public.s_cscf_capabilities OWNER TO icscf;

--
-- Name: s_cscf_capabilities_id_seq; Type: SEQUENCE SET; Schema: public; Owner: icscf
--

SELECT pg_catalog.setval(pg_catalog.pg_get_serial_sequence('s_cscf_capabilities', 'id'), 1, false);


--
-- Name: s_cscf_id_seq; Type: SEQUENCE SET; Schema: public; Owner: icscf
--

SELECT pg_catalog.setval(pg_catalog.pg_get_serial_sequence('s_cscf', 'id'), 1, false);


--
-- Data for Name: nds_trusted_domains; Type: TABLE DATA; Schema: public; Owner: icscf
--

COPY nds_trusted_domains (id, trusted_domain) FROM stdin;
1	open-ims.test
\.


--
-- Data for Name: s_cscf; Type: TABLE DATA; Schema: public; Owner: icscf
--

COPY s_cscf (id, name, s_cscf_uri) FROM stdin;
1	First and only S-CSCF	sip:scscf.open-ims.test:6060
\.


--
-- Data for Name: s_cscf_capabilities; Type: TABLE DATA; Schema: public; Owner: icscf
--

COPY s_cscf_capabilities (id, id_s_cscf, capability) FROM stdin;
1	1	0
2	1	1
\.


--
-- Name: nds_trusted_domains_pkey; Type: CONSTRAINT; Schema: public; Owner: icscf; Tablespace: 
--

ALTER TABLE ONLY nds_trusted_domains
    ADD CONSTRAINT nds_trusted_domains_pkey PRIMARY KEY (id);


--
-- Name: s_cscf_capabilities_pkey; Type: CONSTRAINT; Schema: public; Owner: icscf; Tablespace: 
--

ALTER TABLE ONLY s_cscf_capabilities
    ADD CONSTRAINT s_cscf_capabilities_pkey PRIMARY KEY (id);


--
-- Name: s_cscf_pkey; Type: CONSTRAINT; Schema: public; Owner: icscf; Tablespace: 
--

ALTER TABLE ONLY s_cscf
    ADD CONSTRAINT s_cscf_pkey PRIMARY KEY (id);


--
-- Name: public; Type: ACL; Schema: -; Owner: postgres
--

REVOKE ALL ON SCHEMA public FROM PUBLIC;
REVOKE ALL ON SCHEMA public FROM postgres;
GRANT ALL ON SCHEMA public TO postgres;
GRANT ALL ON SCHEMA public TO PUBLIC;


--
-- PostgreSQL database dump complete
--

