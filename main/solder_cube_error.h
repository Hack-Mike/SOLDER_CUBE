//
// Created by Michele Mocellin on 01/09/2020.
//

#ifndef SOLDER_CUBE_SOLDER_CUBE_ERROR_H
#define SOLDER_CUBE_SOLDER_CUBE_ERROR_H

#endif //SOLDER_CUBE_SOLDER_CUBE_ERROR_H

enum SolderCubeError {
    //custom true/false
    C_FALSE = 1,
    C_TRUE = 0,

    //Serial Error
    FAIL_GET_MAC = 20,
    FAIL_BUILD_SERIAL = 21,
};
