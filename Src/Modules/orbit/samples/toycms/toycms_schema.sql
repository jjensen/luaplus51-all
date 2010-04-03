CREATE TABLE toycms_post
       ("id" INTEGER PRIMARY KEY NOT NULL,
       "title" VARCHAR(255) DEFAULT "",
       "body" TEXT DEFAULT "",
       "abstract" TEXT DEFAULT "",
       "image" VARCHAR(255) DEFAULT "",
       "external_url" VARCHAR(255) DEFAULT "",
       "comment_status" VARCHAR(30) DEFAULT "closed",
       "n_comments" INTEGER DEFAULT 0,
       "section_id" INTEGER DEFAULT NULL,
       "user_id" INTEGER DEFAULT NULL,
       "in_home" BOOLEAN DEFAULT "f",
       "published" BOOLEAN DEFAULT "f",
       "published_at" DATETIME DEFAULT NULL);

CREATE TABLE toycms_comment
       ("id" INTEGER PRIMARY KEY NOT NULL,
       "post_id" INTEGER DEFAULT NULL,
       "author" VARCHAR(255) DEFAULT "",
       "email" VARCHAR(255) DEFAULT "",
       "url" VARCHAR(255) DEFAULT "",
       "body" TEXT DEFAULT "",
       "approved" BOOLEAN DEFAULT "f",
       "created_at" DATETIME DEFAULT NULL);

CREATE TABLE toycms_section
       ("id" INTEGER PRIMARY KEY NOT NULL,
        "title" VARCHAR(255) DEFAULT "",
	"description" TEXT DEFAULT "",
	"tag" VARCHAR(255) DEFAULT "");

CREATE TABLE toycms_user
       ("id" INTEGER PRIMARY KEY NOT NULL,
        "login" VARCHAR(255) DEFAULT "",
	"password" VARCHAR(30) DEFAULT "",
	"name" VARCHAR(255) DEFAULT "");
