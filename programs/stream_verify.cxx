/**
 * Copyright (C) 2014 Virgil Security Inc.
 *
 * Lead Maintainer: Virgil Security Inc. <support@virgilsecurity.com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     (1) Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *     (2) Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *
 *     (3) Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <iostream>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <string>
#include <stdexcept>

#include <virgil/VirgilByteArray.h>
using virgil::VirgilByteArray;

#include <virgil/service/stream/VirgilStreamSigner.h>
using virgil::service::stream::VirgilStreamSigner;

#include <virgil/service/stream/VirgilStreamDataSource.h>
using virgil::service::stream::VirgilStreamDataSource;

#include <virgil/service/data/VirgilSign.h>
using virgil::service::data::VirgilSign;

int print_usage(std::ostream& out, const char *programName) {
    out << "Usage: " << programName << " <data> <sign> <public_key>" << std::endl;
    out << "    <data>       - [in] data file to be verified" << std::endl;
    out << "    <sign>       - [in] file with serialized sign" << std::endl;
    out << "    <public_key> - [in] public key file" << std::endl;
    return -1;
}

int main(int argc, char **argv) {
    // Parse arguments.
    const char *programName = argv[0];
    unsigned currArgPos = 0;

    // Check arguments num.
    if (argc < 4) {
        return print_usage(std::cerr, programName);
    }

    // Parse argument: data
    ++currArgPos;
    std::ifstream dataFile(argv[currArgPos], std::ios::in | std::ios::binary);
    if (!dataFile.is_open()) {
        std::cerr << "Unable to open file: " <<  argv[currArgPos] << std::endl;
        return print_usage(std::cerr, programName);
    }

    // Parse argument: sign
    ++currArgPos;
    std::ifstream signFile(argv[currArgPos], std::ios::in | std::ios::binary);
    if (!signFile.is_open()) {
        std::cerr << "Unable to open file: " <<  argv[currArgPos] << std::endl;
        return print_usage(std::cerr, programName);
    }
    VirgilByteArray signData;
    std::copy(std::istreambuf_iterator<char>(signFile), std::istreambuf_iterator<char>(),
            std::back_inserter(signData));
    signFile.close();

    // Parse argument: public_key
    ++currArgPos;
    std::ifstream publicKeyFile(argv[currArgPos], std::ios::in);
    if (!publicKeyFile.is_open()) {
        std::cerr << "Unable to open file: " <<  argv[currArgPos] << std::endl;
        return print_usage(std::cerr, programName);
    }
    VirgilByteArray publicKey;
    std::copy(std::istreambuf_iterator<char>(publicKeyFile), std::istreambuf_iterator<char>(),
            std::back_inserter(publicKey));
    publicKeyFile.close();

    // Prepare input source.
    VirgilStreamDataSource dataSource(dataFile);

    // Demarshal sign
    bool signDefined = false;
    VirgilSign sign;
    if (!signDefined) {
        try {
            // Demarshal sign from ASN.1.
            sign.fromAsn1(signData);
            signDefined = true;
        } catch (...) {
            // Do nothing. Try next serialization format.
        }
    }
    if (!signDefined) {
        try {
            // Demarshal sign from JSON.
            sign.fromJson(signData);
            signDefined = true;
        } catch (...) {
            // Do nothing. Try next serialization format.
        }
    }

    // Create signer.
    VirgilStreamSigner signer;

    if (signDefined) {
        // Verify data.
        bool verified = signer.verify(dataSource, sign, publicKey);
        std::cout << "Verified: " << (verified ? "YES" : "NO") << std::endl;
    } else {
        std::cout << "Verified failed: Possible malformed sign." << std::endl;
        return print_usage(std::cerr, programName);
    }

    return 0;
}