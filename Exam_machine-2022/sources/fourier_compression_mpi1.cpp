#include <complex>
#include <type_traits>
#include <iostream>
#include <vector>
#include <queue>
#include <cstdint>
#include <algorithm>
#include <chrono>
#include "lodepng/lodepng.h"
#include <mpi.h>

struct sparseSpectralComposition 
{
    std::uint32_t ni, nj;
    std::vector<std::uint32_t> begin_rows;
    std::vector<std::uint32_t> ind_cols;
    std::vector<std::complex<double>> coefficients;
};

std::complex<double>* discretTransformFourier( std::uint32_t width, std::uint32_t height, unsigned char const* pixels, int nbp, int rank )
{
    constexpr const double pi = 3.14159265358979324;
    std::uint32_t ni = height;
    std::uint32_t nj = width;
    std::uint32_t nj_loc = nj / nbp; //nb de lignes prises par chaque processus, on suppose width divisible par nbp
    std::complex<double>* X = new std::complex<double>[ni*nj];
    std::fill(X, X+ni*nj, std::complex<double>(0.,0.));

    for( std::uint32_t k1 = 0; k1 < ni; ++k1 )
    {
        for (std::uint32_t k2 = rank*nj_loc; k2 < (rank+1)*nj_loc; ++k2)
        {
            for (std::uint32_t n2 = 0; n2 < ni; ++n2 )
            {
                std::complex<double> exp2(std::cos(-2*pi*n2*(k2)/height), std::sin(-2*pi*n2*(k2)/height));
                for (std::uint32_t n1 = rank*nj_loc; n1 < (rank+1)*nj_loc; ++n1 )
                {
                    std::complex<double> exp1(std::cos(-2*pi*(n1)*k1/nj), std::sin(-2*pi*(n1)*k1/nj));
                    X[k1*nj+k2] += double(pixels[3*((n1)+n2*nj)])*exp1*exp2;
                }
            }
        }
    }
    std::complex<double>* Xf = new std::complex<double>[ni*nj];
    std::fill(Xf, Xf+ni*nj, std::complex<double>(0.,0.));
    MPI_Allgather(&X, 1, MPI_DOUBLE, Xf, 1, MPI_DOUBLE, MPI_COMM_WORLD);
    return X;
}

sparseSpectralComposition compressSpectralComposition( std::uint32_t width, std::uint32_t height, const std::complex<double>* plainCoefs, double tauxCompression )
{
    std::uint32_t nb_coefs = std::uint32_t(tauxCompression*width*height);
    struct node 
    {
        node(std::uint32_t t_i, std::uint32_t t_j, double val )
            :   i(t_i),
                j(t_j),
                module(val)
        {}

        bool operator < ( node const& v ) const
        {
            return this->module < v.module;
        }

        bool operator > ( node const& v ) const
        {
            return this->module > v.module;
        }

        bool operator < ( double v ) const
        {
            return this->module < v;
        }

        bool operator > ( double v ) const
        {
            return this->module > v;
        }

        std::uint32_t i,j;
        double module;
    };

    std::priority_queue<node,std::vector<node>,std::greater<node>> queue;
    for ( std::size_t i = 0; i < height; ++i )
        for ( std::size_t j = 0; j < width;  ++j )
        {
            if (queue.size() < nb_coefs)
            {
                queue.emplace(i,j,std::abs(plainCoefs[i*width+j]));
            }
            else
            {
                double val = std::abs(plainCoefs[i*width+j]);
                queue.emplace(i,j,val);
                queue.pop();
            }
        }
    //
    std::vector<std::uint32_t> nbCoefsPerRow(height, 0u);
    auto queue2 = queue;
    while (!queue2.empty())
    {
        std::uint32_t i = queue2.top().i;
        nbCoefsPerRow[i] += 1;
        queue2.pop();       
    }
    //
    sparseSpectralComposition sparse;
    sparse.begin_rows.resize(height+1);
    sparse.begin_rows[0] = 0;
    for (std::uint32_t i = 1; i <= height; ++i )
        sparse.begin_rows[i] = sparse.begin_rows[i-1] + nbCoefsPerRow[i-1];
    std::cout << "Nombre de coefficients de fourier restant : " << sparse.begin_rows[height] << std::endl << std::flush;
    sparse.ind_cols.resize(sparse.begin_rows[height]);
    sparse.coefficients.resize(sparse.begin_rows[height]);
    //
    std::fill(nbCoefsPerRow.begin(), nbCoefsPerRow.end(), 0u);
    //
    while (!queue.empty())
    {
        node n = queue.top();
        queue.pop();
        std::uint32_t ind = sparse.begin_rows[n.i]+nbCoefsPerRow[n.i];
        sparse.ind_cols[ind] = n.j;
        sparse.coefficients[ind] = plainCoefs[n.i*width+n.j];
        nbCoefsPerRow[n.i] += 1;
    }
    sparse.ni = height;
    sparse.nj = width;
    return sparse;
}

unsigned char* inversePartialDiscretTransformFourier( sparseSpectralComposition const& sparse )
{
    constexpr const double pi = 3.14159265358979324;
    unsigned char* x = new unsigned char[3*sparse.ni*sparse.nj];
    std::uint32_t ni = sparse.ni;
    std::uint32_t nj = sparse.nj;
    std::fill(x, x+3*ni*nj, 0);


    for (std::uint32_t n1 = 0; n1 < nj; ++n1 )
    {
        for (std::uint32_t n2 = 0; n2 < ni; ++n2 )
        {
            std::complex<double> val(0.);
	    
            for( std::uint32_t k1 = 0; k1 < ni; ++k1 )
            {
                std::complex<double> exp1(std::cos(+2*pi*n1*k1/ni), std::sin(+2*pi*n1*k1/ni));

		
                for (std::uint32_t k2 = sparse.begin_rows[k1]; k2 < sparse.begin_rows[k1+1]; ++k2)
                {
                    std::complex<double> exp2(std::cos(+2*pi*n2*sparse.ind_cols[k2]/nj), std::sin(+2*pi*n2*sparse.ind_cols[k2]/nj));
                    val += sparse.coefficients[k2]*exp1*exp2;
                }
            }
            for (std::uint32_t c = 0; c < 3; ++c )
                x[3*(n1+n2*nj)+c] = static_cast<unsigned char>(val.real()/(ni*nj));
        }
    }
    return x;
}

int main(int nargs, char* argv[])
{
    //on initialise le contexte MPI
    MPI_Init(&nargs, &argv);
    MPI_COMM_WORLD;
    MPI_Comm globComm;
    MPI_Comm_dup(MPI_COMM_WORLD, &globComm);
    int nbp;
    MPI_Comm_size(globComm, &nbp);
    int rank;
    MPI_Comm_rank(globComm, &rank);


    std::chrono::time_point<std::chrono::system_clock> start, mid1, mid2, end;

    std::uint32_t width, height;
    unsigned char* image;
    if (nargs <= 1)
    {
        auto error = lodepng_decode24_file(&image, &width, &height, "data/small_lena_gray.png");
        if(error) std::cerr << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
    }
    else 
    {
        auto error = lodepng_decode24_file(&image, &width, &height, argv[1]);
        if(error) std::cerr << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
    }
    double taux = 0.10; // A changer si on veut pour une image mieux compressée ou de meilleur qualité
    if (nargs > 2)
        taux = std::stod(argv[2]);
    std::cout << "Caracteristique de l'image : " << width << "x" << height << " => " << width*height << " pixels." << std::endl << std::flush;

    start = std::chrono::system_clock::now();
    std::complex<double>* fourier = discretTransformFourier( width, height, image, nbp, rank ); //encodage de l'image avec dft
    mid1 = std::chrono::system_clock::now();
    auto sparse = compressSpectralComposition(width, height, fourier, taux); //selection des p% plus gros coefs
    mid2 = std::chrono::system_clock::now();
    if (rank==0)
    {
    unsigned char* compressed_img = inversePartialDiscretTransformFourier(sparse); //reconstitution de l'image compressee
    
    end = std::chrono::system_clock::now();

    std::chrono::duration<double> temps_encode = mid1 - start;
    std::chrono::duration<double> temps_selec = mid2 - mid1;
    std::chrono::duration<double> temps_reconstit = end - mid2;
 
    std::cout << "Temps encodage image dft = " << temps_encode.count() << std::endl << std::flush;
    std::cout << "Temps selection coefs = " << temps_selec.count() << std::endl << std::flush;
    std::cout << "Temps reconstitution = " << temps_reconstit.count() << std::endl << std::flush;

    auto error = lodepng_encode24_file("compress.png", compressed_img, width, height);
    if(error) std::cerr << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

    std::cout << "Fin du programme..." << std::endl << std::flush;
    delete [] compressed_img;
    delete [] fourier;
    delete [] image;
    }
    MPI_Finalize();
    return EXIT_SUCCESS;
}
