/**
 * Copyright 2019 Christophe Pollet
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef C_VM_VIDEO_H
#define C_VM_VIDEO_H

typedef struct Video Video;

Video *video_create(bool enable);

void video_loop(Video *this);

void video_pause(Video *this);

void video_stop(Video *this);

void video_destroy(Video *self);

#endif //C_VM_VIDEO_H
