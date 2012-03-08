/***************************************************************************
 *   Copyright (C) 2008                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef CACHE_H
#define CACHE_H

#include <QDateTime>

#define MAX_SEARCH_CACHE 10000
#define MAX_CHUNK_CACHE 1000

typedef struct cachehash
{
  char *host;
  char *name;
  char *hash;
  qint64 size;
  QDateTime *time;
  QString *comment;
} CHASH;

typedef struct cachechunk
{
  char *host;
  char *name;
  int chunkNumber;
  long size;
  unsigned char *data;
  QDateTime *time;
} CCHUNK;

extern CHASH *Cache;
extern int maxCache;
extern int nbCache;

extern CCHUNK *CacheChunk;
extern int maxCacheChunk;
extern int nbCacheChunk;

void startCache();
void stopCache();

#endif
